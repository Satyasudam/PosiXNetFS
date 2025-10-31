#ifndef IPC_H
#define IPC_H

#include "protocol.h" // For MAX_PATH_LEN

#define SYNC_SOCKET_PATH "/tmp/dcfs_sync_socket"
#define MAX_IPC_MSG_LEN 512

typedef enum {
    IPC_SYNC_FILE,      // A file has been written and needs to be uploaded
    IPC_DELETE_FILE,    // A file was deleted locally
    IPC_STATUS_REQ,
    IPC_STATUS_RES
} ipc_msg_type_t;

typedef struct {
    ipc_msg_type_t type;
    char path[MAX_PATH_LEN];
    char status_message[MAX_IPC_MSG_LEN - MAX_PATH_LEN - sizeof(ipc_msg_type_t)];
} ipc_message_t;

#endif // IPC_H

