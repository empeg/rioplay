LIBMAD  = libmad/libmad.a
LIBFLAC = libflac/libFLAC.a
LIBVORBISIDEC = libvorbisidec/libvorbisidec.a

Decoders.o: Decoder.o Mp3Decoder.o FlacDecoder.o VorbisDecoder.o
	$(LD) -r -o Decoders.o Decoder.o Mp3Decoder.o FlacDecoder.o VorbisDecoder.o \
        $(LIBMAD) $(LIBFLAC) $(LIBVORBISIDEC)

Decoder.o: Decoder.cc Decoder.hh
	$(C++) -c -o Decoder.o $(CFLAGS) Decoder.cc $(INCLUDEDIRS)

Mp3Decoder.o: Mp3Decoder.cc Mp3Decoder.hh
	$(C++) -c -o Mp3Decoder.o $(CFLAGS) Mp3Decoder.cc $(INCLUDEDIRS)

FlacDecoder.o: FlacDecoder.cc FlacDecoder.hh
	$(C++) -c -o FlacDecoder.o $(CFLAGS) FlacDecoder.cc $(INCLUDEDIRS)

VorbisDecoder.o: VorbisDecoder.cc VorbisDecoder.hh
	$(C++) -c -o VorbisDecoder.o $(CFLAGS) VorbisDecoder.cc $(INCLUDEDIRS)
