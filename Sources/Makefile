Sources.o: InputSource.o RioServerSource.o ShoutcastSource.o EmpegSource.o
	$(LD) -r -o Sources.o InputSource.o RioServerSource.o ShoutcastSource.o EmpegSource.o
    
InputSource.o: InputSource.cc InputSource.hh
	$(C++) -c -o InputSource.o $(CFLAGS) $(INCLUDEDIRS) InputSource.cc

RioServerSource.o: RioServerSource.cc RioServerSource.hh
	$(C++) -c -o RioServerSource.o $(CFLAGS) $(INCLUDEDIRS) RioServerSource.cc

ShoutcastSource.o: ShoutcastSource.cc ShoutcastSource.hh
	$(C++) -c -o ShoutcastSource.o $(CFLAGS) $(INCLUDEDIRS) ShoutcastSource.cc

EmpegSource.o: EmpegSource.cc EmpegSource.hh
	$(C++) -c -o EmpegSource.o $(CFLAGS) $(INCLUDEDIRS) EmpegSource.cc
