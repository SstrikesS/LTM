all: server client
server: server.c
	gcc -pthread -o server server.c
client: client.c
	gcc -o client client.c
clean:
	rm -f *.o *~ server client