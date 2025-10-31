#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "../common/protocol.h"
#include "../common/utils.h"
#include "server_worker.h"
#include "storage.h"
#include "metadata.h"

int main(int argc, char *argv[]) {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);
    pthread_t thread_id;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <storage_root_path>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    storage_init(argv[1]);
    metadata_init();
    LOG_INFO("Storage initialized at %s", argv[1]);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        LOG_ERROR("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        LOG_ERROR("setsockopt failed");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        LOG_ERROR("Bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 10) < 0) {
        LOG_ERROR("Listen failed");
        exit(EXIT_FAILURE);
    }

    LOG_INFO("Server listening on port %d", PORT);

    while ((new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen))) {
        if (new_socket < 0) {
            LOG_ERROR("Accept failed");
            continue;
        }
        LOG_INFO("Connection accepted");
        
        int *new_sock = malloc(sizeof(int));
        *new_sock = new_socket;

        if (pthread_create(&thread_id, NULL, handle_client, (void*) new_sock) < 0) {
            LOG_ERROR("Could not create thread for client");
            free(new_sock);
            close(new_socket);
        } else {
            pthread_detach(thread_id);
        }
    }

    if (new_socket < 0) {
        LOG_ERROR("Accept loop terminated");
        exit(EXIT_FAILURE);
    }

    close(server_fd);
    metadata_destroy();
    return 0;
}

