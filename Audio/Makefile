Audio.o: AudioOutputDevice.o RioReceiverAudio.o
	$(LD) -r -o Audio.o AudioOutputDevice.o RioReceiverAudio.o

AudioOutputDevice.o: AudioOutputDevice.cc AudioOutputDevice.hh
	$(C++) -c -o AudioOutputDevice.o $(CFLAGS) AudioOutputDevice.cc $(INCLUDEDIRS)

RioReceiverAudio.o: RioReceiverAudio.cc RioReceiverAudio.hh
	$(C++) -c -o RioReceiverAudio.o $(CFLAGS) RioReceiverAudio.cc $(INCLUDEDIRS)
