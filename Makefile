CC=gcc

CFLAGS= -pthread -Iinclude
LDFLAGS = -lssl -lcrypto -lpthread
TARGET=build/main
SRCS= src/main.c src/playground.c src/admin.c src/server.c src/client.c src/user.c src/room.c src/custom_error.c

all:
	mkdir -p build
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS)
    
run: all
	./$(TARGET)

clean: 
	rm -rf build
