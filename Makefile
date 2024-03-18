CC=gcc -g
#CFLAGS= -std=c99 -Wall

COR: main.o select.o udp_functions.o tcp_functions.o protocol_msgs.o user_cmds.o aux_functions.o
	$(CC) -o COR main.o select.o udp_functions.o tcp_functions.o protocol_msgs.o user_cmds.o aux_functions.o

main.o: main.c select.h protocol_msgs.h global_variables.h
	$(CC) -c main.c

select.o: select.c select.h udp_functions.h tcp_functions.h
	$(CC) -c select.c

udp_functions.o: udp_functions.c udp_functions.h
	$(CC) -c udp_functions.c

tcp_functions.o: tcp_functions.c tcp_functions.h
	$(CC) -c tcp_functions.c

user_cmds.o: user_cmds.c user_cmds.h udp_functions.h tcp_functions.h global_variables.h aux_functions.h
	$(CC) -c user_cmds.c

protocol_msgs.o: protocol_msgs.c protocol_msgs.h udp_functions.h global_variables.h
	$(CC) -c protocol_msgs.c

aux_functions.o: aux_functions.c aux_functions.h global_variables.h
	$(CC) -c aux_functions.c

clean::
	rm -f *.o core a.out client server *~