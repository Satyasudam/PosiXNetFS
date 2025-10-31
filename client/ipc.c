#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <errno.h>
#include "../common/ipc.h"
#include "../common/utils.h"

void ipc_notify_sync(const char *path) {
    int s;
    struct sockaddr_un remote;
    ipc_message_t msg;

    if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        LOG_ERROR("IPC socket create failed: %s", strerror(errno));
        return;
    }

    remote.sun_family = AF_UNIX;
    strcpy(remote.sun_path, SYNC_SOCKET_PATH);
    socklen_t len = strlen(remote.sun_path) + sizeof(remote.sun_family);
    
    if (connect(s, (struct sockaddr *)&remote, len) == -1) {
        LOG_ERROR("IPC connect failed. Is sync_daemon running? Error: %s", strerror(errno));
        close(s);
        return;
    }

    memset(&msg, 0, sizeof(msg));
    msg.type = IPC_SYNC_FILE;
    strncpy(msg.path, path, sizeof(msg.path) - 1);
    
    if (send(s, &msg, sizeof(msg), 0) == -1) {
        LOG_ERROR("IPC send failed: %s", strerror(errno));
    } else {
        LOG_DEBUG("Sent sync request for %s", path);
    }

    close(s);
}


