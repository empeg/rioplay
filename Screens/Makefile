INCLUDEDIRS = -I../Main/ -I../Fonts/ -I../Utils/

Screens.o: Screen.o StatusScreen.o LogoScreen.o MenuScreen.o
	ld -r -o Screens.o Screen.o StatusScreen.o LogoScreen.o MenuScreen.o
    
Screen.o: Screen.cc Screen.hh LetterBitmaps.h NumbersBitmaps.h
	g++ -c -o Screen.o $(CFLAGS) Screen.cc
        
StatusScreen.o: StatusScreen.cc StatusScreen.hh Screen.hh
	g++ -c -o StatusScreen.o $(CFLAGS) $(INCLUDEDIRS) StatusScreen.cc
        
LogoScreen.o: LogoScreen.cc LogoScreen.hh Screen.hh
	g++ -c -o LogoScreen.o $(CFLAGS) $(INCLUDEDIRS) LogoScreen.cc

MenuScreen.o: MenuScreen.cc MenuScreen.hh Screen.hh
	g++ -c -o MenuScreen.o $(CFLAGS) $(INCLUDEDIRS) MenuScreen.cc
