Remote.o: RemoteThread.o
	ld -r -o Remote.o RemoteThread.o
    
RemoteThread.o: RemoteThread.cc RemoteThread.hh
	$(C++) -c -o RemoteThread.o $(CFLAGS) RemoteThread.cc $(INCLUDEDIRS)
