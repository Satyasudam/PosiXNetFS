CC=gcc
CFLAGS=-I../common -Wall -pthread
LDFLAGS=-pthread
TARGET=dcfs_server
SRCS=server_main.c server_worker.c storage.c metadata.c auth.c server_utils.c
OBJS=$(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

