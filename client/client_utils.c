#include "../common/utils.h"
#include <string.h>
#include <errno.h>
#include <libgen.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>

// This file shares the implementation from the common directory,
// but we need to provide the object file for linking.
// The primary client-specific utility is mkdir_p.
void dcfs_log(const char *level, const char *file, int line, const char *fmt, ...) {
    va_list args;
    char time_buf[64];
    time_t t = time(NULL);
    
    time_buf[strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", localtime(&t))] = '\0';

    fprintf(stderr, "[%s] [%s] [%s:%d] ", time_buf, level, file, line);
    
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    
    fprintf(stderr, "\n");
    fflush(stderr);
}
int mkdir_p(const char *path) {
    char *path_copy = strdup(path);
    if (!path_copy) {
        LOG_ERROR("strdup failed");
        return -1;
    }
    
    char *p = path_copy;
    // Skip leading slashes
    while (*p == '/') p++;

    while ((p = strchr(p, '/'))) {
        *p = '\0'; // Temporarily terminate the string
        if (mkdir(path_copy, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) {
            if (errno != EEXIST) {
                LOG_ERROR("mkdir(%s) failed: %s", path_copy, strerror(errno));
                free(path_copy);
                return -1;
            }
        }
        *p = '/'; // Restore the slash
        p++;
    }
    
    // Create the final component of the path
    if (mkdir(path_copy, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) {
        if (errno != EEXIST) {
            LOG_ERROR("mkdir(%s) failed: %s", path_copy, strerror(errno));
            free(path_copy);
            return -1;
        }
    }

    free(path_copy);
    return 0;
}


