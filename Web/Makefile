Web.o: WebThread.o
	ld -r -o Web.o WebThread.o
    
WebThread.o: WebThread.cc WebThread.hh
	$(C++) -c -o WebThread.o $(CFLAGS) WebThread.cc $(INCLUDEDIRS)
