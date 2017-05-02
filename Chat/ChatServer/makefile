
BIN:=bins
DEBUG:=debug
SRC:=src

chatserver:$(DEBUG)/link.o $(DEBUG)/mythread.o $(DEBUG)/server.o
	gcc -o $(BIN)/chatserver $(DEBUG)/link.o $(DEBUG)/mythread.o $(DEBUG)/server.o -W -Wall -lpthread
$(DEBUG)/link.o:$(SRC)/link.c |$(DEBUG)
	gcc -o $(DEBUG)/link.o -c $(SRC)/link.c -W -Wall -lpthread
$(DEBUG)/mythread.o:$(SRC)/mythread.c |$(DEBUG)
	gcc -o $(DEBUG)/mythread.o -c $(SRC)/mythread.c -W -Wall -lpthread
$(DEBUG)/server.o:$(SRC)/server.c |$(DEBUG)
	gcc -o $(DEBUG)/server.o -c $(SRC)/server.c -W -Wall -lpthread

$(DEBUG):
	mkdir $@

.PHONY:clean
clean:
	rm -f $(DEBUG)/*.o  
	rm -f $(BIN)/chatserver
