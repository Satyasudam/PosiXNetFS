#ifndef STORAGE_H
#define STORAGE_H

#include <sys/stat.h>
#include <sys/statvfs.h>
#include <unistd.h>
#include "../common/protocol.h"

// Initializes the storage backend at the given root path.
void storage_init(const char *root);

// Constructs the full absolute path for a given relative file path.
void storage_get_fullpath(const char *path, char *fullpath, size_t size);

// Gets file attributes.
int storage_getattr(const char *path, struct stat *stbuf);

// Opens a file (for permission checking, etc.).
int storage_open(const char *path, int flags);

// Reads data from a file.
ssize_t storage_read(const char *path, char *buf, size_t size, off_t offset);

// Writes data to a file.
ssize_t storage_write(const char *path, const char *buf, size_t size, off_t offset);

// Creates a new file.
int storage_create(const char *path, mode_t mode);

// Deletes a file.
int storage_unlink(const char *path);

// Creates a directory.
int storage_mkdir(const char *path, mode_t mode);

// Removes a directory.
int storage_rmdir(const char *path);

// Gets filesystem statistics.
int storage_statfs(const char *path, struct statvfs *stbuf);

// Truncates a file to a specified size.
int storage_truncate(const char *path, off_t size);

// Renames a file or directory.
int storage_rename(const char* from, const char* to);

#endif // STORAGE_H
