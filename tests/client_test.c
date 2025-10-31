#include <stdio.h>
#include <stdlib.h>
#include "../client/client_net.h"
#include "../common/protocol.h"

int main() {
    printf("Starting client network test...\n");

    if (net_connect("127.0.0.1") != 0) {
        fprintf(stderr, "Test failed: Could not connect.\n");
        return 1;
    }
    printf("Connection: PASS\n");

    if (net_authenticate("user", "password") != 0) {
        fprintf(stderr, "Test failed: Authentication failed.\n");
        net_disconnect();
        return 1;
    }
    printf("Authentication: PASS\n");
    
    response_t res;
    if(net_getattr("/", &res) == 0) {
        printf("getattr('/'): PASS\n");
    } else {
        printf("getattr('/'): FAIL\n");
    }

    net_disconnect();
    printf("Disconnected.\n");
    
    printf("Client network test finished successfully.\n");
    return 0;
}

