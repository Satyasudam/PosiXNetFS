#include "../common/utils.h"
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <libgen.h>

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
    while (*p == '/') p++;

    while ((p = strchr(p, '/'))) {
        *p = '\0';
        if (mkdir(path_copy, S_IRWXU) != 0) {
            if (errno != EEXIST) {
                LOG_ERROR("mkdir(%s) failed: %s", path_copy, strerror(errno));
                free(path_copy);
                return -1;
            }
        }
        *p = '/';
        p++;
    }
    
    if (mkdir(path_copy, S_IRWXU) != 0) {
        if (errno != EEXIST) {
            LOG_ERROR("mkdir(%s) failed: %s", path_copy, strerror(errno));
            free(path_copy);
            return -1;
        }
    }

    free(path_copy);
    return 0;
}

