#Abbas: Compiler and flags for building position-independent code
CC = gcc
CFLAGS = -Wall -Wextra -Iinclude -fPIC

#Abbas: Linker flags for dynamic linking with shared library
LDFLAGS = -L. -lgame -lpthread -Wl,-rpath,.

#Abbas: Shared library name
LIB = libgame.so

#Abbas: Build shared library and both executables
all: $(LIB) server client

#Abbas: Build shared library from modular source files
$(LIB): src/game.c src/questions.c src/session.c src/protocol.c src/csapp.c
	$(CC) $(CFLAGS) -shared -o $(LIB) src/game.c src/questions.c src/session.c src/protocol.c src/csapp.c

#Abbas: Build server and link dynamically with shared library
server: src/server/server.c $(LIB)
	$(CC) -Wall -Wextra -Iinclude src/server/server.c -o server $(LDFLAGS)

#Abbas Build client and link dynamically with shared library
client: src/client/client.c $(LIB)
	$(CC) -Wall -Wextra -Iinclude src/client/client.c -o client $(LDFLAGS)

#Abbas: Remove generated executables and shared library
clean:
	rm -f server client $(LIB) *.o

#Abbas: Rebuild project from scratch
rebuild: clean all
