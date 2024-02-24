CC=gcc
#CFLAGS= -std=c99 -Wall

chat: main.o udp_functions.o tcp_functions.o
	$(CC) -o chat main.o udp_functions.o tcp_functions.o

main.o: main.c udp_functions.h tcp_functions.h
	$(CC) -c main.c

udp_functions.o: udp_functions.c udp_functions.h
	$(CC) -c udp_functions.c

tcp_functions.o: tcp_functions.c tcp_functions.h
	$(CC) -c tcp_functions.c

clean::
	rm -f *.o core a.out main *~