Fonts.o: Font.o SmallFont.o
	$(LD) -r -o Fonts.o Font.o SmallFont.o
    
Font.o: Font.cc Font.hh
	$(C++) -c -o Font.o $(CFLAGS) $(INCLUDEDIRS) Font.cc
        
SmallFont.o: SmallFont.cc SmallFont.hh Font.hh SmallFontBitmap.h
	$(C++) -c -o SmallFont.o $(CFLAGS) $(INCLUDEDIRS) SmallFont.cc
