LIBMAD = ../../mad-0.14.2b/libmad/.libs/libmad.a

Audio.o: AudioThread.o MadCallbacks.o
	ld -r -o Audio.o AudioThread.o MadCallbacks.o $(LIBMAD)

AudioThread.o: AudioThread.cc AudioThread.hh mad.h MadCallbacks.hh
	$(C++) -c -o AudioThread.o $(CFLAGS) AudioThread.cc $(INCLUDEDIRS)

MadCallbacks.o: MadCallbacks.cc MadCallbacks.hh mad.h
	$(C++) -c -o MadCallbacks.o $(CFLAGS) MadCallbacks.cc $(INCLUDEDIRS)
