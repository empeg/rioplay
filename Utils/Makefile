Utils.o: Http.o
	ld -r -o Utils.o Http.o
    
Http.o: Http.c Http.h
	$(C++) -c -o Http.o $(CFLAGS) $(INCLUDEDIRS) Http.c
