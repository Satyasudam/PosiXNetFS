#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include "server_worker.h"
#include "../common/protocol.h"
#include "../common/utils.h"
#include "storage.h"
#include "metadata.h"
#include "auth.h"

static void process_request(int sock, request_t *req, int *is_authenticated);
void *handle_client(void* socket_desc);
static void process_request(int sock, request_t *req, int *is_authenticated);

void *handle_client(void *socket_desc) {
    int sock = *(int*)socket_desc;
    free(socket_desc);
    request_t req;
    ssize_t read_size;
    int is_authenticated = 0;

    while ((read_size = recv(sock, &req, sizeof(req), 0)) > 0) {
        process_request(sock, &req, &is_authenticated);
    }

    if (read_size == 0) {
        LOG_INFO("Client disconnected gracefully.");
    } else if (read_size == -1) {
        LOG_ERROR("recv failed: %s", strerror(errno));
    }

    close(sock);
    return NULL;
}

static void process_request(int sock, request_t *req, int *is_authenticated) {
    response_t res;
    memset(&res, 0, sizeof(res));

    if (req->type != REQ_AUTH && !*is_authenticated) {
        LOG_WARN("Unauthenticated request type %d received", req->type);
        res.type = RES_ERROR;
        res.error = EACCES;
        send(sock, &res, sizeof(res), 0);
        return;
    }

    LOG_DEBUG("Request type: %d, path: %s", req->type, req->path);
    metadata_lock_file(req->path);

    switch (req->type) {
        case REQ_AUTH:
            if (auth_user(req->auth.username, req->auth.password)) {
                *is_authenticated = 1;
                res.type = RES_OK;
                LOG_INFO("User '%s' authenticated successfully", req->auth.username);
            } else {
                res.type = RES_ERROR;
                res.error = EACCES;
                LOG_WARN("User '%s' authentication failed", req->auth.username);
            }
            break;
        case REQ_GETATTR:
            res.error = storage_getattr(req->path, &res.st);
            res.type = res.error == 0 ? RES_ATTR : RES_ERROR;
            break;
        case REQ_READDIR:
            {
                char full_path[MAX_PATH_LEN];
                storage_get_fullpath(req->path, full_path, sizeof(full_path));
                DIR *dp = opendir(full_path);
                if (dp == NULL) {
                    res.type = RES_ERROR;
                    res.error = errno;
                } else {
                    struct dirent *de;
                    while ((de = readdir(dp)) != NULL) {
                        res.type = RES_DIR;
                        strncpy((char*)res.data, de->d_name, MAX_DATA_LEN - 1);
                        res.size = strlen(de->d_name) + 1;
                        send(sock, &res, sizeof(res), 0);
                    }
                    closedir(dp);
                    res.type = RES_DIR;
                    res.size = 0;
                }
            }
            break;
        case REQ_OPEN:
            res.error = storage_open(req->path, req->flags);
            res.type = res.error >= 0 ? RES_OK : RES_ERROR;
            if (res.error < 0) res.error = -res.error;
            break;
        case REQ_READ:
            res.size = storage_read(req->path, (char*)res.data, req->size, req->offset);
            if ((int)res.size < 0) {
                res.type = RES_ERROR;
                res.error = -res.size;
                res.size = 0;
            } else {
                res.type = RES_DATA;
            }
            break;
        case REQ_WRITE:
            res.size = storage_write(req->path, (const char*)req->data, req->size, req->offset);
            if ((int)res.size < 0) {
                res.type = RES_ERROR;
                res.error = -res.size;
                res.size = 0;
            } else {
                res.type = RES_OK;
            }
            //metadata_update_version(req->path);
            break;
        case REQ_CREATE:
            res.error = storage_create(req->path, req->mode);
            res.type = res.error == 0 ? RES_OK : RES_ERROR;
            if(res.error < 0) res.error = -res.error;
            break;
        case REQ_UNLINK:
            res.error = storage_unlink(req->path);
            res.type = res.error == 0 ? RES_OK : RES_ERROR;
            break;
        case REQ_MKDIR:
            res.error = storage_mkdir(req->path, req->mode);
            res.type = res.error == 0 ? RES_OK : RES_ERROR;
            break;
        case REQ_RMDIR:
            res.error = storage_rmdir(req->path);
            res.type = res.error == 0 ? RES_OK : RES_ERROR;
            break;
        case REQ_RELEASE:
            res.type = RES_OK;
            break;
        case REQ_STATFS:
             res.error = storage_statfs(req->path, &res.stfs);
             res.type = res.error == 0 ? RES_STATFS_DATA : RES_ERROR;
             break;
        case REQ_TRUNCATE:
             res.error = storage_truncate(req->path, req->offset);
             res.type = res.error == 0 ? RES_OK : RES_ERROR;
             break;
        case REQ_RENAME:
             res.error = storage_rename(req->path, req->new_path);
             res.type = res.error == 0 ? RES_OK : RES_ERROR;
             break;
        default:
            res.type = RES_ERROR;
            res.error = EINVAL;
            LOG_WARN("Invalid request type: %d", req->type);
            break;
    }
    
    metadata_unlock_file(req->path);
    if (req->type != REQ_READDIR) {
        send(sock, &res, sizeof(res), 0);
    }
}

