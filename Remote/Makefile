Remote.o: RemoteThread.o CommandHandler.o
	$(LD) -r -o Remote.o RemoteThread.o CommandHandler.o
    
RemoteThread.o: RemoteThread.cc RemoteThread.hh
	$(C++) -c -o RemoteThread.o $(CFLAGS) RemoteThread.cc $(INCLUDEDIRS)

CommandHandler.o: CommandHandler.cc CommandHandler.hh
	$(C++) -c -o CommandHandler.o $(CFLAGS) CommandHandler.cc $(INCLUDEDIRS)
