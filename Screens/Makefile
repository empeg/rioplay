Screens.o: Screen.o StatusScreen.o LogoScreen.o MenuScreen.o
	$(LD) -r -o Screens.o Screen.o StatusScreen.o LogoScreen.o MenuScreen.o

Screen.o: Screen.cc Screen.hh
	$(C++) -c -o Screen.o $(CFLAGS) $(INCLUDEDIRS) Screen.cc
    
StatusScreen.o: StatusScreen.cc StatusScreen.hh Screen.hh
	$(C++) -c -o StatusScreen.o $(CFLAGS) $(INCLUDEDIRS) StatusScreen.cc
        
LogoScreen.o: LogoScreen.cc LogoScreen.hh Screen.hh
	$(C++) -c -o LogoScreen.o $(CFLAGS) $(INCLUDEDIRS) LogoScreen.cc

MenuScreen.o: MenuScreen.cc MenuScreen.hh Screen.hh
	$(C++) -c -o MenuScreen.o $(CFLAGS) $(INCLUDEDIRS) MenuScreen.cc
