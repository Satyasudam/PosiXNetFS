#include "auth.h"
#include <string.h>

int auth_user(const char* username, const char* password) {
    if (strcmp(username, "user") == 0 && strcmp(password, "password") == 0) {
        return 1;
    }
    if (strcmp(username, "admin") == 0 && strcmp(password, "adminpass") == 0) {
        return 1;
    }
    return 0;
}

