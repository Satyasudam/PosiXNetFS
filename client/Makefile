CC=gcc
# Add FUSE compiler flags globally. This ensures the compiler knows where to find <fuse.h>.
CFLAGS=-I../common -I. -Wall -pthread `pkg-config fuse3 --cflags`

# Add FUSE linker flags for linking the final executables.
LDFLAGS=-pthread `pkg-config fuse3 --libs`

# Source files for the client modules (shared code without a main() function)
MODULE_SRCS = \
	ipc.c \
	cache.c \
	client_net.c \
	client_utils.c

MODULE_OBJS = $(MODULE_SRCS:.c=.o)

# The executables we want to build
TARGETS = dcfs_client sync_daemon

all: $(TARGETS)

# Rule to build the main FUSE client executable
dcfs_client: fuse_main.o $(MODULE_OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Rule to build the background sync daemon executable
sync_daemon: sync_daemon.o $(MODULE_OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Generic pattern rule to build any .o file from its corresponding .c file.
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to clean up all compiled files
clean:
	rm -f $(TARGETS) *.o


