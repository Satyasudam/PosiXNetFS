#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "../server/storage.h"
#include "../server/auth.h"

int main() {
    printf("Starting server components test...\n");

    if (auth_user("user", "password") && !auth_user("user", "wrong")) {
        printf("Auth test: PASS\n");
    } else {
        printf("Auth test: FAIL\n");
    }

    storage_init("/tmp/dcfs_test_root");
    printf("Storage initialized.\n");
    
    if(storage_mkdir("/testdir", 0755) == 0) {
        printf("Storage mkdir: PASS\n");
    } else {
        printf("Storage mkdir: FAIL\n");
    }

    const char* content = "hello world";
    if(storage_write("/testdir/test.txt", content, strlen(content), 0) > 0) {
        printf("Storage write: PASS\n");
    } else {
        printf("Storage write: FAIL\n");
    }

    struct stat st;
    if(storage_getattr("/testdir/test.txt", &st) == 0) {
        printf("Storage getattr: PASS\n");
    } else {
        printf("Storage getattr: FAIL\n");
    }

    printf("Server components test finished.\n");
    return 0;
}

