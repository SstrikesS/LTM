all: server_forking chat_client
server_forking: linklist.o server.c
	gcc linklist.o server.c -o server_forking
chat_client: client.c
	gcc client.c -o chat_client
linklist.o: linklist.h linklist.c
	gcc linklist.h -c linklist.c
clean:
	rm -f *.o *~ linklist.h.gch