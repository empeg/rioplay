Sources.o: InputSource.o RioServerSource.o ShoutcastSource.o
	$(LD) -r -o Sources.o InputSource.o RioServerSource.o ShoutcastSource.o
    
InputSource.o: InputSource.cc InputSource.hh
	$(C++) -c -o InputSource.o $(CFLAGS) $(INCLUDEDIRS) InputSource.cc

RioServerSource.o: RioServerSource.cc RioServerSource.hh
	$(C++) -c -o RioServerSource.o $(CFLAGS) $(INCLUDEDIRS) RioServerSource.cc

ShoutcastSource.o: ShoutcastSource.cc ShoutcastSource.hh
	$(C++) -c -o ShoutcastSource.o $(CFLAGS) $(INCLUDEDIRS) ShoutcastSource.cc
