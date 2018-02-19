CFLAGS = gcc -Wall

build: server client

server: server.c
	$(CFLAGS) server.c -o server
	
client: client.c
	$(CFLAGS) client.c -o client

.PHONY: clean run_server run_client

clean:
	rm -f server client
