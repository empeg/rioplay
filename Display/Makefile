Display.o: DisplayThread.o
	ld -r -o Display.o DisplayThread.o
    
DisplayThread.o: DisplayThread.cc DisplayThread.hh
	g++ -c -o DisplayThread.o $(CFLAGS) $(INCLUDEDIRS) DisplayThread.cc
