#ifndef CACHE_H
#define CACHE_H

#include <sys/types.h>
#include <unistd.h>

void cache_init(const char* cache_dir);
int cache_has_file(const char* path);
int cache_read_file(const char* path, char* buf, size_t size, off_t offset);
int cache_write_file(const char* path, const char* data, size_t size);
int cache_pwrite_file(const char* path, const char* data, size_t size, off_t offset);
int cache_create_file(const char *path, mode_t mode);
int cache_delete_file(const char* path);
int cache_create_directory(const char *path, mode_t mode);
int cache_delete_directory(const char* path);
int cache_truncate_file(const char *path, off_t size);
int cache_rename_file(const char *from, const char *to);

#endif // CACHE_H


