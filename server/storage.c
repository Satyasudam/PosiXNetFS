#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include "storage.h"
#include "../common/utils.h"

static char storage_root[MAX_PATH_LEN];

void storage_init(const char *root) {
    strncpy(storage_root, root, sizeof(storage_root) - 1);
    storage_root[sizeof(storage_root) - 1] = '\0';
    mkdir(storage_root, 0755);
}

void storage_get_fullpath(const char *path, char *fullpath, size_t size) {
    snprintf(fullpath, size, "%s%s", storage_root, path);
}

int storage_getattr(const char *path, struct stat *stbuf) {
    char fullpath[MAX_PATH_LEN];
    storage_get_fullpath(path, fullpath, sizeof(fullpath));
    if (lstat(fullpath, stbuf) == -1) return -errno;
    return 0;
}

int storage_open(const char *path, int flags) {
    char fullpath[MAX_PATH_LEN];
    storage_get_fullpath(path, fullpath, sizeof(fullpath));
    int fd = open(fullpath, flags);
    if (fd == -1) return -errno;
    close(fd);
    return 0;
}

ssize_t storage_read(const char *path, char *buf, size_t size, off_t offset) {
    char fullpath[MAX_PATH_LEN];
    storage_get_fullpath(path, fullpath, sizeof(fullpath));
    int fd = open(fullpath, O_RDONLY);
    if (fd == -1) return -errno;
    ssize_t res = pread(fd, buf, size, offset);
    close(fd);
    return res == -1 ? -errno : res;
}

ssize_t storage_write(const char *path, const char *buf, size_t size, off_t offset) {
    char fullpath[MAX_PATH_LEN];
    storage_get_fullpath(path, fullpath, sizeof(fullpath));
    int fd = open(fullpath, O_WRONLY | O_CREAT, 0644);
    if (fd == -1) return -errno;
    ssize_t res = pwrite(fd, buf, size, offset);
    close(fd);
    return res == -1 ? -errno : res;
}

int storage_create(const char *path, mode_t mode) {
    char fullpath[MAX_PATH_LEN];
    storage_get_fullpath(path, fullpath, sizeof(fullpath));
    int fd = creat(fullpath, mode);
    if (fd == -1) return -errno;
    close(fd);
    return 0;
}

int storage_unlink(const char *path) {
    char fullpath[MAX_PATH_LEN];
    storage_get_fullpath(path, fullpath, sizeof(fullpath));
    return unlink(fullpath) == -1 ? -errno : 0;
}

int storage_mkdir(const char *path, mode_t mode) {
    char fullpath[MAX_PATH_LEN];
    storage_get_fullpath(path, fullpath, sizeof(fullpath));
    return mkdir(fullpath, mode) == -1 ? -errno : 0;
}

int storage_rmdir(const char *path) {
    char fullpath[MAX_PATH_LEN];
    storage_get_fullpath(path, fullpath, sizeof(fullpath));
    return rmdir(fullpath) == -1 ? -errno : 0;
}

int storage_statfs(const char *path, struct statvfs *stbuf) {
    return statvfs(storage_root, stbuf) == -1 ? -errno : 0;
}

int storage_truncate(const char *path, off_t size) {
    char fullpath[MAX_PATH_LEN];
    storage_get_fullpath(path, fullpath, sizeof(fullpath));
    return truncate(fullpath, size) == -1 ? -errno : 0;
}

int storage_rename(const char* from, const char* to) {
    char fullpath_from[MAX_PATH_LEN];
    char fullpath_to[MAX_PATH_LEN];
    storage_get_fullpath(from, fullpath_from, sizeof(fullpath_from));
    storage_get_fullpath(to, fullpath_to, sizeof(fullpath_to));
    return rename(fullpath_from, fullpath_to) == -1 ? -errno : 0;
}

