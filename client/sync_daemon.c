#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

#include "../common/ipc.h"
#include "../common/utils.h"
#include "client_net.h"
#include "cache.h"

static void main_loop(int listen_fd);
static void handle_ipc_connection(int sock);
static void process_sync_request(const char* path);

int main(void) {
    int listen_fd;
    struct sockaddr_un local;

    if (net_connect("127.0.0.1") != 0 || net_authenticate("user", "password") != 0) {
        LOG_ERROR("Daemon failed to connect or auth with server. Exiting.");
        exit(EXIT_FAILURE);
    }
    LOG_INFO("Sync daemon is connected and authenticated with the server.");

    cache_init("/tmp/dcfs_cache");

    if ((listen_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        LOG_ERROR("Daemon IPC socket create failed: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    local.sun_family = AF_UNIX;
    strcpy(local.sun_path, SYNC_SOCKET_PATH);
    unlink(local.sun_path);
    socklen_t len = strlen(local.sun_path) + sizeof(local.sun_family);

    if (bind(listen_fd, (struct sockaddr *)&local, len) == -1) {
        LOG_ERROR("Daemon IPC socket bind failed: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (listen(listen_fd, 10) == -1) {
        LOG_ERROR("Daemon IPC socket listen failed: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    LOG_INFO("Sync daemon is listening for local IPC on %s", SYNC_SOCKET_PATH);

    main_loop(listen_fd);
    
    close(listen_fd);
    net_disconnect();
    return 0;
}

static void main_loop(int listen_fd) {
    int conn_fd;
    struct sockaddr_un remote;
    socklen_t remote_len = sizeof(remote);

    while ((conn_fd = accept(listen_fd, (struct sockaddr *)&remote, &remote_len)) != -1) {
        LOG_DEBUG("Daemon accepted a new IPC connection.");
        handle_ipc_connection(conn_fd);
    }

    if (conn_fd == -1) {
        LOG_ERROR("Daemon accept loop failed: %s", strerror(errno));
    }
}

static void handle_ipc_connection(int sock) {
    ipc_message_t msg;
    ssize_t n = recv(sock, &msg, sizeof(msg), 0);
    
    if (n <= 0) {
        if (n < 0) LOG_ERROR("Daemon IPC recv error: %s", strerror(errno));
        else LOG_WARN("Daemon received empty IPC message.");
        close(sock);
        return;
    }

    if (msg.type == IPC_SYNC_FILE) {
        LOG_INFO("Daemon received sync task for path: %s", msg.path);
        process_sync_request(msg.path);
    } else {
        LOG_WARN("Daemon received unknown IPC message type: %d", msg.type);
    }
    
    close(sock);
}

static void process_sync_request(const char* path) {
    char cache_path[MAX_PATH_LEN];
    snprintf(cache_path, sizeof(cache_path), "/tmp/dcfs_cache%s", path);

    int fd = open(cache_path, O_RDONLY);
    if (fd == -1) {
        LOG_ERROR("SYNC: Failed to open cache file %s: %s", cache_path, strerror(errno));
        return;
    }

    LOG_DEBUG("SYNC: Starting upload for %s", path);
    
    char buffer[MAX_DATA_LEN];
    ssize_t bytes_read;
    off_t offset = 0;
    response_t res;

    while ((bytes_read = pread(fd, buffer, sizeof(buffer), offset)) > 0) {
        if (net_write(path, buffer, bytes_read, offset, &res) < 0) {
            LOG_ERROR("SYNC: Failed to write chunk to server for %s (offset %ld)", path, offset);
            break;
        }
        offset += bytes_read;
    }
    
    if (bytes_read < 0) {
        LOG_ERROR("SYNC: Failed to read from cache file %s: %s", cache_path, strerror(errno));
    } else {
        LOG_INFO("SYNC: Successfully completed upload for file %s", path);
    }
    
    close(fd);
}


