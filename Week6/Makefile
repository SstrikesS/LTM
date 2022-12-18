all: server client
server: linklist.o server.c
	gcc linklist.o server.c -o server
client: client.c
	gcc client.c -o client
linklist.o: linklist.h linklist.c
	gcc linklist.h -c linklist.c
clean:
	rm -f *.o *~ linklist.h.gch