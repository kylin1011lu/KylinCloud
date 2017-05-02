
BIN:=bin
DEBUG:=debug
SRC:=src

server:link.o mythread.o server.o
	gcc -o $(BIN)/server $(DEBUG)/link.o $(DEBUG)/mythread.o $(DEBUG)/server.o -W -Wall -lpthread
link.o:$(SRC)/link.c
	gcc -o $(DEBUG)/link.o -c $(SRC)/link.c -W -Wall -lpthread
mythread.o:$(SRC)/mythread.c
	gcc -o $(DEBUG)/mythread.o -c $(SRC)/mythread.c -W -Wall -lpthread
server.o:$(SRC)/server.c
	gcc -o $(DEBUG)/server.o -c $(SRC)/server.c -W -Wall -lpthread


.PHONY:clean
clean:
	rm -f $(DEBUG)/*.o  
	rm -f $(BIN)/server
