C++ = g++
INCLUDEDIRS = -I../Main/ -I../Utils/ -I../Screens/ -I../Display/ -I../Audio/ -I../Fonts/ -I../Main/ -I../Remote/

Playlists.o: Playlist.o RioServerList.o ShoutcastList.o
	ld -r -o Playlists.o Playlist.o RioServerList.o ShoutcastList.o
    
Playlist.o: Playlist.cc Playlist.hh
	$(C++) -c -o Playlist.o $(CFLAGS) $(INCLUDEDIRS) Playlist.cc

RioServerList.o: RioServerList.cc RioServerList.hh
	$(C++) -c -o RioServerList.o $(CFLAGS) $(INCLUDEDIRS) RioServerList.cc

ShoutcastList.o: ShoutcastList.cc ShoutcastList.hh
	$(C++) -c -o ShoutcastList.o $(CFLAGS) $(INCLUDEDIRS) ShoutcastList.cc
