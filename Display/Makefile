Display.o: DisplayThread.o
	ld -r -o Display.o DisplayThread.o
    
DisplayThread.o: DisplayThread.cc DisplayThread.hh
	$(C++) -c -o DisplayThread.o $(CFLAGS) $(INCLUDEDIRS) DisplayThread.cc
