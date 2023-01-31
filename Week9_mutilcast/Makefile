all: mcast test test2
mcast: mcast.c
	gcc -pthread -Wall -o mcast mcast.c
test : test.c
	gcc -Wall -pthread -o test test.c
test2 : test2.c
	gcc -Wall -o test2 test2.c
clean:
	rm -f *.o *~ mcast test test2