Main.o: Start.o Thread.o
	ld -r -o Main.o Start.o Thread.o

Start.o: Start.cc Player.h
	g++ -c -o Start.o $(CFLAGS) $(INCLUDEDIRS) Start.cc
    
Thread.o: Thread.cc Thread.hh
	g++ -c -o Thread.o $(CFLAGS) Thread.cc
