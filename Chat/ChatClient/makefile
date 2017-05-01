BIN:=bin
DEBUG:=debug
SRC:=src

client:cfun.o client.o
	gcc -o $(BIN)/client $(DEBUG)/cfun.o $(DEBUG)/client.o -W -Wall -lpthread
cfun.o:$(SRC)/cfun.c
	gcc -o $(DEBUG)/cfun.o -c $(SRC)/cfun.c -W -Wall -lpthread
client.o:$(SRC)/client.c
	gcc -o $(DEBUG)/client.o -c $(SRC)/client.c -W -Wall -lpthread

.PHONY:clean
clean:
	rm -f $(DEBUG)/*.o 
	rm -f $(BIN)/client
