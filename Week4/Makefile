all: server client
server: linklist.o server.c
	gcc -Wall linklist.o server.c -o server
client: client.c
	gcc -o client client.c
linklist.o: linklist.h linklist.c
	gcc -Wall linklist.h -c linklist.c
clean:
	rm -f *.o *~