LIBMAD  = libmad/libmad.a
LIBFLAC = libflac/libFLAC.a

Decoders.o: Decoder.o Mp3Decoder.o FlacDecoder.o
	$(LD) -r -o Decoders.o Decoder.o Mp3Decoder.o FlacDecoder.o \
        $(LIBMAD) $(LIBFLAC)

Decoder.o: Decoder.cc Decoder.hh
	$(C++) -c -o Decoder.o $(CFLAGS) Decoder.cc $(INCLUDEDIRS)

Mp3Decoder.o: Mp3Decoder.cc Mp3Decoder.hh
	$(C++) -c -o Mp3Decoder.o $(CFLAGS) Mp3Decoder.cc $(INCLUDEDIRS)

FlacDecoder.o: FlacDecoder.cc FlacDecoder.hh
	$(C++) -c -o FlacDecoder.o $(CFLAGS) FlacDecoder.cc $(INCLUDEDIRS)
