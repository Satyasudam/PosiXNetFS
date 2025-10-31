#ifndef CLIENT_NET_H
#define CLIENT_NET_H

#define FUSE_USE_VERSION 31
#include <fuse3/fuse.h>
#include "../common/protocol.h"
#include <sys/statvfs.h>

int net_connect(const char* server_ip);
void net_disconnect();
int net_authenticate(const char* user, const char* pass);
int net_getattr(const char *path, response_t* res);
int net_readdir(const char *path, void *buf, fuse_fill_dir_t filler);
int net_open(const char *path, int flags);
int net_read(const char *path, char *buf, size_t size, off_t offset, response_t* res);
int net_write(const char *path, const char *buf, size_t size, off_t offset, response_t* res);
int net_create(const char *path, mode_t mode);
int net_unlink(const char *path);
int net_mkdir(const char *path, mode_t mode);
int net_rmdir(const char *path);
int net_truncate(const char *path, off_t size);
int net_rename(const char *from, const char *to);
int net_statfs(const char *path, struct statvfs *stbuf);

#endif // CLIENT_NET_H


