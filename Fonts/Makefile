Fonts.o: Font.o SmallFont.o
	ld -r -o Fonts.o Font.o SmallFont.o
    
Font.o: Font.cc Font.hh
	g++ -c -o Font.o $(CFLAGS) Font.cc
        
SmallFont.o: SmallFont.cc SmallFont.hh Font.hh SmallFontBitmap.h
	g++ -c -o SmallFont.o $(CFLAGS) SmallFont.cc
