Display.o: DisplayThread.o
	$(LD) -r -o Display.o DisplayThread.o
    
DisplayThread.o: DisplayThread.cc DisplayThread.hh Stopwatch.h
	$(C++) -c -o DisplayThread.o $(CFLAGS) $(INCLUDEDIRS) DisplayThread.cc
