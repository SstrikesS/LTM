CFLAGS = -c -Wall
CC = gcc

all: user_management

user_management: user_management.o
	${CC} user_management.o -o user_management
user_management.o:
	${CC} ${CFLAGS} user_management.c
clean:
	rm -f *.o *~