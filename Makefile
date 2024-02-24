CC=gcc
#CFLAGS= -std=c99 -Wall

chat: client.o udp_functions.o tcp_functions.o
	$(CC) -o client client.o udp_functions.o tcp_functions.o
	$(CC) -o server server.o udp_functions.o tcp_functions.o	

client.o: client.c udp_functions.h tcp_functions.h
	$(CC) -c client.c

server.o: server.c udp_functions.h tcp_functions.h
	$(CC) -c server.c

udp_functions.o: udp_functions.c udp_functions.h
	$(CC) -c udp_functions.c

tcp_functions.o: tcp_functions.c tcp_functions.h
	$(CC) -c tcp_functions.c

clean::
	rm -f *.o core a.out client server *~