Utils.o: Http.o BufferClass.o MemAlloc.o
	ld -r -o Utils.o Http.o BufferClass.o MemAlloc.o
    
Http.o: Http.cc Http.hh
	$(C++) -c -o Http.o $(CFLAGS) $(INCLUDEDIRS) Http.cc

MemAlloc.o: MemAlloc.cc MemAlloc.hh
	$(C++) -c -o MemAlloc.o $(CFLAGS) $(INCLUDEDIRS) MemAlloc.cc

BufferClass.o: BufferClass.cc BufferClass.hh
	$(C++) -c -o BufferClass.o $(CFLAGS) $(INCLUDEDIRS) BufferClass.cc
