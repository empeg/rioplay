Audio.o: AudioOutputDevice.o
	$(LD) -r -o Audio.o AudioOutputDevice.o

AudioOutputDevice.o: AudioOutputDevice.cc AudioOutputDevice.hh
	$(C++) -c -o AudioOutputDevice.o $(CFLAGS) AudioOutputDevice.cc $(INCLUDEDIRS)
