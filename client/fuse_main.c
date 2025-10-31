#define FUSE_USE_VERSION 31

#include <fuse3/fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <sys/stat.h>
#include <sys/statvfs.h>

#include "client_net.h"
#include "cache.h"
#include "ipc.h"
#include "../common/utils.h"
#include "../common/protocol.h"

static int dcfs_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi) {
    (void)fi;
    response_t res;
    int ret = net_getattr(path, &res);
    if (ret == 0) {
        memcpy(stbuf, &res.st, sizeof(struct stat));
    }
    return ret;
}

static int dcfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi, enum fuse_readdir_flags flags) {
    (void)offset; (void)fi; (void)flags;
    return net_readdir(path, buf, filler);
}

static int dcfs_open(const char *path, struct fuse_file_info *fi) {
    if (!cache_has_file(path)) {
        LOG_INFO("File %s not in cache, downloading.", path);
        response_t res;
        char buffer[MAX_DATA_LEN];
        int ret = net_read(path, buffer, MAX_DATA_LEN, 0, &res);
        if (ret != 0 && ret != -ENOENT) return ret;
        
        if (res.size > 0) {
            if (cache_write_file(path, buffer, res.size) < 0) return -EIO;
        }
    }
    return net_open(path, fi->flags);
}

static int dcfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    (void)fi;
    return cache_read_file(path, buf, size, offset);
}

static int dcfs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    (void)fi;
    int res = cache_pwrite_file(path, buf, size, offset);
    if (res >= 0) {
        ipc_notify_sync(path);
    }
    return res;
}

static int dcfs_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    (void)fi;
    int res = net_create(path, mode);
    if (res == 0) {
        cache_create_file(path, mode);
        ipc_notify_sync(path);
    }
    return res;
}

static int dcfs_unlink(const char *path) {
    int res = net_unlink(path);
    if (res == 0) {
        cache_delete_file(path);
    }
    return res;
}

static int dcfs_mkdir(const char *path, mode_t mode) {
    int res = net_mkdir(path, mode);
    if (res == 0) {
        cache_create_directory(path, mode);
    }
    return res;
}

static int dcfs_rmdir(const char *path) {
    int res = net_rmdir(path);
    if (res == 0) {
        cache_delete_directory(path);
    }
    return res;
}

static int dcfs_truncate(const char *path, off_t size, struct fuse_file_info *fi) {
    (void)fi;
    int res = net_truncate(path, size);
    if (res == 0) {
        cache_truncate_file(path, size);
        ipc_notify_sync(path);
    }
    return res;
}

static int dcfs_rename(const char *from, const char *to, unsigned int flags) {
    (void)flags;
    int res = net_rename(from, to);
    if (res == 0) {
        cache_rename_file(from, to);
    }
    return res;
}

static int dcfs_statfs(const char *path, struct statvfs *stbuf) {
    return net_statfs(path, stbuf);
}

static const struct fuse_operations dcfs_ops = {
    .getattr    = dcfs_getattr,
    .readdir    = dcfs_readdir,
    .open       = dcfs_open,
    .read       = dcfs_read,
    .write      = dcfs_write,
    .create     = dcfs_create,
    .unlink     = dcfs_unlink,
    .mkdir      = dcfs_mkdir,
    .rmdir      = dcfs_rmdir,
    .truncate   = dcfs_truncate,
    .rename     = dcfs_rename,
    .statfs     = dcfs_statfs,
};

int main(int argc, char *argv[]) {
    cache_init("/tmp/dcfs_cache");
    if (net_connect("127.0.0.1") != 0) {
        fprintf(stderr, "Failed to connect to server.\n");
        return 1;
    }
    if (net_authenticate("user", "password") != 0) {
        fprintf(stderr, "Authentication failed.\n");
        net_disconnect();
        return 1;
    }
    
    LOG_INFO("Authenticated and connected. Mounting filesystem.");
    return fuse_main(argc, argv, &dcfs_ops, NULL);
}


