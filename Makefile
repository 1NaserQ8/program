CC = gcc
CFLAGS = -Wall -Wextra -Iinclude -fPIC
LDFLAGS = -L. -lgame -lpthread -Wl,-rpath,.
LIB = libgame.so

all: $(LIB) server client

$(LIB): src/game.c src/questions.c src/session.c src/protocol.c src/csapp.c
	$(CC) $(CFLAGS) -shared -o $(LIB) src/game.c src/questions.c src/session.c src/protocol.c src/csapp.c

server: src/server/server.c $(LIB)
	$(CC) -Wall -Wextra -Iinclude src/server/server.c -o server $(LDFLAGS)

client: src/client/client.c $(LIB)
	$(CC) -Wall -Wextra -Iinclude src/client/client.c -o client $(LDFLAGS)

clean:
	rm -f server client $(LIB) *.o

rebuild: clean all
