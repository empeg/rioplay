Web.o: WebMain.o
	ld -r -o Web.o WebMain.o
    
WebMain.o: WebMain.c
	gcc -c -o WebMain.o $(CFLAGS) WebMain.c -I../Main/
