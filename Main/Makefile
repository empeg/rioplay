Main.o: Start.o Thread.o
	$(LD) -r -o Main.o Start.o Thread.o

Start.o: Start.cc Player.h
	$(C++) -c -o Start.o $(CFLAGS) $(INCLUDEDIRS) Start.cc
    
Thread.o: Thread.cc Thread.hh
	$(C++) -c -o Thread.o $(CFLAGS) $(INCLUDEDIRS) Thread.cc
