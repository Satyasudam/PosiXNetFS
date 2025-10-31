#ifndef SERVER_WORKER_H
#define SERVER_WORKER_H

/**
 * @brief The entry point for a new thread that handles a single client connection.
 * * @param socket_desc A pointer to the client's socket file descriptor.
 * @return void* Returns NULL when the connection is closed.
 */
void *handle_client(void *socket_desc);

#endif // SERVER_WORKER_H
