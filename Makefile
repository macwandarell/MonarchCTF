CC=gcc

CFLAGS= -pthread -Iinclude `pkg-config fuse3 --cflags` -D_FILE_OFFSET_BITS=64 -DFUSE_USE_VERSION=31
LDFLAGS = -lssl -lcrypto -lpthread -lutil `pkg-config fuse3 --libs`
TARGET=build/main
SRCS= src/main.c src/playground.c src/admin.c src/server.c src/client.c src/user.c src/room.c src/fuser.c src/custom_error.c

all:
	mkdir -p build
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS)
    
run: all
	./$(TARGET)

clean: 
	rm -rf build
