#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <libgen.h>

#include "cache.h"
#include "../common/utils.h"
#define MAX_PATH_LEN 1024
static char cache_root[MAX_PATH_LEN];

static void get_cache_path(const char *path, char *cache_path, size_t size) {
    snprintf(cache_path, size, "%s%s", cache_root, path);
}

void cache_init(const char* cache_dir) {
    strncpy(cache_root, cache_dir, sizeof(cache_root) - 1);
    cache_root[sizeof(cache_root) - 1] = '\0';
    if (mkdir(cache_root, 0755) != 0 && errno != EEXIST) {
        LOG_ERROR("Failed to create cache directory %s: %s", cache_root, strerror(errno));
    } else {
        LOG_INFO("Cache initialized at %s", cache_root);
    }
}

int cache_has_file(const char* path) {
    char cache_path[MAX_PATH_LEN];
    get_cache_path(path, cache_path, sizeof(cache_path));
    return access(cache_path, F_OK) == 0;
}

int cache_read_file(const char* path, char* buf, size_t size, off_t offset) {
    char cache_path[MAX_PATH_LEN];
    get_cache_path(path, cache_path, sizeof(cache_path));
    int fd = open(cache_path, O_RDONLY);
    if (fd == -1) return -errno;
    int res = pread(fd, buf, size, offset);
    close(fd);
    return res < 0 ? -errno : res;
}

int cache_write_file(const char* path, const char* data, size_t size) {
    char cache_path[MAX_PATH_LEN];
    get_cache_path(path, cache_path, sizeof(cache_path));
    
    char *dir_path_copy = strdup(cache_path);
    mkdir_p(dirname(dir_path_copy));
    free(dir_path_copy);
    
    int fd = open(cache_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) return -errno;
    int res = write(fd, data, size);
    close(fd);
    return res < 0 ? -errno : res;
}

int cache_pwrite_file(const char* path, const char* data, size_t size, off_t offset) {
    char cache_path[MAX_PATH_LEN];
    get_cache_path(path, cache_path, sizeof(cache_path));

    char *dir_path_copy = strdup(cache_path);
    mkdir_p(dirname(dir_path_copy));
    free(dir_path_copy);

    int fd = open(cache_path, O_WRONLY | O_CREAT, 0644);
    if (fd == -1) return -errno;
    int res = pwrite(fd, data, size, offset);
    close(fd);
    return res < 0 ? -errno : res;
}

int cache_create_file(const char *path, mode_t mode) {
    char cache_path[MAX_PATH_LEN];
    get_cache_path(path, cache_path, sizeof(cache_path));
    int fd = creat(cache_path, mode);
    if (fd == -1) return -errno;
    close(fd);
    return 0;
}

int cache_delete_file(const char* path) {
    char cache_path[MAX_PATH_LEN];
    get_cache_path(path, cache_path, sizeof(cache_path));
    return unlink(cache_path) == -1 ? -errno : 0;
}

int cache_create_directory(const char *path, mode_t mode) {
    char cache_path[MAX_PATH_LEN];
    get_cache_path(path, cache_path, sizeof(cache_path));
    return mkdir(cache_path, mode) == -1 ? -errno : 0;
}

int cache_delete_directory(const char* path) {
    char cache_path[MAX_PATH_LEN];
    get_cache_path(path, cache_path, sizeof(cache_path));
    return rmdir(cache_path) == -1 ? -errno : 0;
}

int cache_truncate_file(const char *path, off_t size) {
    char cache_path[MAX_PATH_LEN];
    get_cache_path(path, cache_path, sizeof(cache_path));
    return truncate(cache_path, size) == -1 ? -errno : 0;
}

int cache_rename_file(const char *from, const char *to) {
    char cache_from[MAX_PATH_LEN];
    char cache_to[MAX_PATH_LEN];
    get_cache_path(from, cache_from, sizeof(cache_from));
    get_cache_path(to, cache_to, sizeof(cache_to));
    return rename(cache_from, cache_to) == -1 ? -errno : 0;
}


