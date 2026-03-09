all:
	gcc src/server/server.c src/csapp.c -Iinclude -o server -lpthread
	gcc src/client/client.c src/csapp.c -Iinclude -o client -lpthread

server:
	gcc src/server/server.c src/csapp.c -Iinclude -o server -lpthread

client:
	gcc src/client/client.c src/csapp.c -Iinclude -o client -lpthread

clean:
	rm -f server client

rebuild: clean all