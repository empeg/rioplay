Web.o: WebThread.o
	$(LD) -r -o Web.o WebThread.o
    
WebThread.o: WebThread.cc WebThread.hh
	$(C++) -c -o WebThread.o $(CFLAGS) WebThread.cc $(INCLUDEDIRS)
