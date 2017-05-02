BIN:=bins
DEBUG:=debug
SRC:=src


chatclient:$(DEBUG)/cfun.o $(DEBUG)/client.o
	gcc -o $(BIN)/chatclient $(DEBUG)/cfun.o $(DEBUG)/client.o -W -Wall -lpthread
$(DEBUG)/cfun.o:$(SRC)/cfun.c | $(DEBUG)
	gcc -o $(DEBUG)/cfun.o -c $(SRC)/cfun.c -W -Wall -lpthread
$(DEBUG)/client.o:$(SRC)/client.c | $(DEBUG)
	gcc -o $(DEBUG)/client.o -c $(SRC)/client.c -W -Wall -lpthread


$(DEBUG):
	mkdir $@

.PHONY:clean
clean:
	rm -f $(DEBUG)/*.o 
	rm -f $(BIN)/chatclient
