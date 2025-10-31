#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statvfs.h>

#define PORT 8080
#define MAX_PATH_LEN 256
#define MAX_DATA_LEN 4096
#define MAX_USERNAME_LEN 64
#define MAX_PASSWORD_LEN 64

// Request types from client to server
typedef enum {
    REQ_AUTH,
    REQ_GETATTR,
    REQ_READDIR,
    REQ_OPEN,
    REQ_READ,
    REQ_WRITE,
    REQ_CREATE,
    REQ_UNLINK,
    REQ_MKDIR,
    REQ_RMDIR,
    REQ_RELEASE,
    REQ_STATFS,
    REQ_TRUNCATE,
    REQ_RENAME
} request_type_t;

// Response types from server to client
typedef enum {
    RES_OK,
    RES_ERROR,
    RES_DATA,
    RES_ATTR,
    RES_DIR,
    RES_STATFS_DATA
} response_type_t;

// Authentication request
typedef struct {
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];
} auth_request_t;

// Generic request structure
typedef struct {
    request_type_t type;
    char path[MAX_PATH_LEN];
    char new_path[MAX_PATH_LEN]; // For rename
    int flags;
    mode_t mode;
    off_t offset;
    size_t size;
    uint8_t data[MAX_DATA_LEN];
    auth_request_t auth;
} request_t;

// Generic response structure
typedef struct {
    response_type_t type;
    int error;
    struct stat st;
    size_t size;
    uint8_t data[MAX_DATA_LEN];
    struct statvfs stfs;
} response_t;

#endif // PROTOCOL_H

