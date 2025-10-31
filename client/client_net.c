#define FUSE_USE_VERSION 31

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>

#include "client_net.h"
#include "../common/utils.h"

typedef struct DirEntry {
    char name[MAX_DATA_LEN];
    struct DirEntry *next;
} DirEntry;

static int sock = -1;
static pthread_mutex_t net_mutex = PTHREAD_MUTEX_INITIALIZER;

static int perform_request(request_t *req, response_t *res) {
    pthread_mutex_lock(&net_mutex);
    if (send(sock, req, sizeof(*req), 0) < 0) {
        LOG_ERROR("Network send failed: %s", strerror(errno));
        pthread_mutex_unlock(&net_mutex);
        return -EIO;
    }
    if (recv(sock, res, sizeof(*res), 0) <= 0) {
        LOG_ERROR("Network recv failed: %s", strerror(errno));
        pthread_mutex_unlock(&net_mutex);
        return -EIO;
    }
    pthread_mutex_unlock(&net_mutex);
    return res->type == RES_ERROR ? -res->error : 0;
}

int net_connect(const char* server_ip) {
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        LOG_ERROR("Socket creation error");
        return -1;
    }
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        LOG_ERROR("Invalid address/ Address not supported");
        return -1;
    }
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        LOG_ERROR("Connection Failed: %s", strerror(errno));
        return -1;
    }
    LOG_INFO("Connected to server %s", server_ip);
    return 0;
}

void net_disconnect() {
    close(sock);
}

int net_authenticate(const char* user, const char* pass) {
    request_t req;
    response_t res;
    memset(&req, 0, sizeof(req));
    req.type = REQ_AUTH;
    strncpy(req.auth.username, user, sizeof(req.auth.username)-1);
    strncpy(req.auth.password, pass, sizeof(req.auth.password)-1);
    return perform_request(&req, &res);
}

int net_getattr(const char *path, response_t* res) {
    request_t req;
    memset(&req, 0, sizeof(req));
    req.type = REQ_GETATTR;
    strncpy(req.path, path, sizeof(req.path)-1);
    return perform_request(&req, res);
}

int net_readdir(const char *path, void *buf, fuse_fill_dir_t filler) {
    request_t req;
    response_t res;
    memset(&req, 0, sizeof(req));
    req.type = REQ_READDIR;
    strncpy(req.path, path, sizeof(req.path)-1);

    DirEntry *head = NULL, *current = NULL;
    int ret = 0;

    pthread_mutex_lock(&net_mutex);
    if (send(sock, &req, sizeof(req), 0) < 0) {
        LOG_ERROR("Readdir send failed: %s", strerror(errno));
        ret = -EIO;
    } else {
        while(1) {
            if (recv(sock, &res, sizeof(res), 0) <= 0) {
                LOG_ERROR("Readdir recv failed: %s", strerror(errno));
                ret = -EIO;
                break;
            }
            if (res.type == RES_ERROR) {
                ret = -res.error;
                break;
            }
            if (res.type == RES_DIR && res.size == 0) break;
            
            DirEntry *new_entry = malloc(sizeof(DirEntry));
            if (!new_entry) { ret = -ENOMEM; break; }
            strcpy(new_entry->name, (const char*)res.data);
            new_entry->next = NULL;
            if (!head) head = current = new_entry;
            else { current->next = new_entry; current = new_entry; }
        }
    }
    pthread_mutex_unlock(&net_mutex);

    if (ret != 0) {
        while (head) { current = head; head = head->next; free(current); }
        return ret;
    }
    
    filler(buf, ".", NULL, 0, 0);
    filler(buf, "..", NULL, 0, 0);
    
    current = head;
    while(current) {
        if (strcmp(current->name, ".") != 0 && strcmp(current->name, "..") != 0) {
             filler(buf, current->name, NULL, 0, 0);
        }
        DirEntry *to_free = current;
        current = current->next;
        free(to_free);
    }
    return 0;
}

int net_open(const char *path, int flags) {
    request_t req;
    response_t res;
    memset(&req, 0, sizeof(req));
    req.type = REQ_OPEN;
    strncpy(req.path, path, sizeof(req.path) - 1);
    req.flags = flags;
    return perform_request(&req, &res);
}

int net_read(const char *path, char *buf, size_t size, off_t offset, response_t* res) {
    request_t req;
    memset(&req, 0, sizeof(req));
    req.type = REQ_READ;
    strncpy(req.path, path, sizeof(req.path) - 1);
    req.size = size > MAX_DATA_LEN ? MAX_DATA_LEN : size;
    req.offset = offset;
    int ret = perform_request(&req, res);
    if (ret == 0 && res->size > 0) {
        memcpy(buf, res->data, res->size);
    }
    return ret;
}

int net_write(const char *path, const char *buf, size_t size, off_t offset, response_t* res) {
    request_t req;
    memset(&req, 0, sizeof(req));
    req.type = REQ_WRITE;
    strncpy(req.path, path, sizeof(req.path) - 1);
    size_t write_size = size > MAX_DATA_LEN ? MAX_DATA_LEN : size;
    memcpy(req.data, buf, write_size);
    req.size = write_size;
    req.offset = offset;
    return perform_request(&req, res);
}

int net_create(const char *path, mode_t mode) {
    request_t req;
    response_t res;
    memset(&req, 0, sizeof(req));
    req.type = REQ_CREATE;
    strncpy(req.path, path, sizeof(req.path)-1);
    req.mode = mode;
    return perform_request(&req, &res);
}

int net_unlink(const char *path) {
    request_t req;
    response_t res;
    memset(&req, 0, sizeof(req));
    req.type = REQ_UNLINK;
    strncpy(req.path, path, sizeof(req.path)-1);
    return perform_request(&req, &res);
}

int net_mkdir(const char *path, mode_t mode) {
    request_t req;
    response_t res;
    memset(&req, 0, sizeof(req));
    req.type = REQ_MKDIR;
    strncpy(req.path, path, sizeof(req.path)-1);
    req.mode = mode;
    return perform_request(&req, &res);
}

int net_rmdir(const char *path) {
    request_t req;
    response_t res;
    memset(&req, 0, sizeof(req));
    req.type = REQ_RMDIR;
    strncpy(req.path, path, sizeof(req.path)-1);
    return perform_request(&req, &res);
}

int net_truncate(const char *path, off_t size) {
    request_t req;
    response_t res;
    memset(&req, 0, sizeof(req));
    req.type = REQ_TRUNCATE;
    strncpy(req.path, path, sizeof(req.path)-1);
    req.offset = size; // Using offset to carry the size
    return perform_request(&req, &res);
}

int net_rename(const char *from, const char *to) {
    request_t req;
    response_t res;
    memset(&req, 0, sizeof(req));
    req.type = REQ_RENAME;
    strncpy(req.path, from, sizeof(req.path)-1);
    strncpy(req.new_path, to, sizeof(req.new_path)-1);
    return perform_request(&req, &res);
}

int net_statfs(const char *path, struct statvfs *stbuf) {
    request_t req;
    response_t res;
    memset(&req, 0, sizeof(req));
    req.type = REQ_STATFS;
    strncpy(req.path, path, sizeof(req.path)-1);
    
    int ret = perform_request(&req, &res);
    if (ret == 0) {
        memcpy(stbuf, &res.stfs, sizeof(struct statvfs));
    }
    return ret;
}


