Display.o: DisplayThread.o VFDLib.o
	$(LD) -r -o Display.o DisplayThread.o VFDLib.o
    
DisplayThread.o: DisplayThread.cc DisplayThread.hh Stopwatch.h
	$(C++) -c -o DisplayThread.o $(CFLAGS) $(INCLUDEDIRS) DisplayThread.cc

VFDLib.o: VFDLib.cc VFDLib.hh
	$(C++) -c -o VFDLib.o $(CFLAGS) $(INCLUDEDIRS) VFDLib.cc
