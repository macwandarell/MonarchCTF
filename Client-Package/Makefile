CC=gcc


CFLAGS=-Iinclude
LDFLAGS= -lssl -lcrypto
TARGET=build/main
SRCS= src/main.c src/client.c src/custom_error.c

all:
	mkdir -p build
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)  $(LDFLAGS)
    
run: all
	./$(TARGET)

clean: 
	rm -rf build
