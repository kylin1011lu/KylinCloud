client:cfun.o client.o
	gcc -o client cfun.o client.o -Wall -lpthread
cfun.o:cfun.c
	gcc -o cfun.o -c cfun.c -Wall -lpthread
client.o:client.c
	gcc -o client.o -c client.c -Wall -lpthread

.PHONY:clean
clean:
	rm *.o *.c *.h 
	rm client_update.tar
	rm makefile
