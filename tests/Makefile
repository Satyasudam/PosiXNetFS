CC=gcc
# Add include paths for headers in common, server, and client directories
CFLAGS=-I../common -I../server -I../client -Wall -pthread
LDFLAGS=-pthread

# --- Server Test ---
# List the server .c files needed for the test.
# We exclude server_main.c because it has its own main() function.
SERVER_MODULE_SRCS = \
	../server/storage.c \
	../server/auth.c \
	../server/metadata.c \
	../server/server_utils.c \
	../server/server_worker.c

# --- Client Test ---
# List the client .c files needed for the test.
# Exclude files with a main() function like fuse_main.c and sync_daemon.c
CLIENT_MODULE_SRCS = \
	../client/ipc.c \
	../client/cache.c \
	../client/client_net.c \
	../client/client_utils.c

# Add FUSE flags specifically for the client test
CLIENT_CFLAGS = $(CFLAGS) `pkg-config fuse3 --cflags`
CLIENT_LDFLAGS = $(LDFLAGS) `pkg-config fuse3 --libs`

TARGETS=server_test client_test

all: $(TARGETS)

# Rule to build the server test.
# It compiles server_test.c AND all the server modules together in one command.
server_test: server_test.c $(SERVER_MODULE_SRCS)
# IMPORTANT: The next line MUST start with a TAB character, not spaces.
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Rule to build the client test.
# It compiles client_test.c AND all the client modules together.
client_test: client_test.c $(CLIENT_MODULE_SRCS)
# IMPORTANT: The next line MUST start with a TAB character, not spaces.
	$(CC) $(CLIENT_CFLAGS) $^ -o $@ $(CLIENT_LDFLAGS)

clean:
# IMPORTANT: The next line MUST start with a TAB character, not spaces.
	rm -f $(TARGETS) *.o
