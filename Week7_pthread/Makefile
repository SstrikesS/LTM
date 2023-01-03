all: server_thread chat_client
server_thread: linklist.o server.c
	gcc linklist.o server.c -pthread -o server_thread
chat_client: client.c
	gcc client.c -pthread -o chat_client
linklist.o: linklist.h linklist.c
	gcc linklist.h -c linklist.c
clean:
	rm -f *.o *~ linklist.h.gch server_thread chat_client