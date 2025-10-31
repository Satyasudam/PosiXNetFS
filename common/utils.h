#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

// Generic logging function
void dcfs_log(const char *level, const char *file, int line, const char *fmt, ...);

// Macros for easy logging
#define LOG_INFO(...) dcfs_log("INFO", __FILE__, __LINE__, __VA_ARGS__)
#define LOG_WARN(...) dcfs_log("WARN", __FILE__, __LINE__, __VA_ARGS__)
#define LOG_ERROR(...) dcfs_log("ERROR", __FILE__, __LINE__, __VA_ARGS__)
#define LOG_DEBUG(...) dcfs_log("DEBUG", __FILE__, __LINE__, __VA_ARGS__)

// Utility to create parent directories, like `mkdir -p`
int mkdir_p(const char *path);

#endif // UTILS_H

