// Rioplay - Replacement Rio Receiver player software
// Copyright (C) 2002 David Flowerday
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// version 2 as published by the Free Software Foundation.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

#ifndef MP3DECODER_HH
#define MP3DECODER_HH

#include <stdio.h>
#include "libmad/mad.h"
#include "Thread.hh"
#include "StatusScreen.hh"
#include "AudioOutputDevice.hh"
#include "InputSource.hh"
#include "Decoder.hh"

#define BUFFER_SIZE 16384

class BufferClass;

class Mp3Decoder : public Decoder {
public:
    Mp3Decoder(int inInputFD, AudioOutputDevice *inAudioDev, InputSource *inPList);
    ~Mp3Decoder(void);
    virtual void *ThreadMain(void *arg);
    void SetMetadataFrequency(int Freq);
    
    /* These have to be public so the C jump functions can access them.
       They should not be used outside this class. */
    enum mad_flow InputCallback(void *ptr, struct mad_stream *stream);
    enum mad_flow OutputCallback(void *ptr, struct mad_header const *header, struct mad_pcm *pcm);
    enum mad_flow ErrorCallback(void *ptr, struct mad_stream *stream, struct mad_frame *frame);
    
private:
    unsigned char *Buffer;
    mad_timer_t CurrentTime;
    struct mad_decoder MadDecoder;
    int MetadataFrequency;
    int FirstRun;
    char *LocalBuffer;
    int BufferSize;
    
};

#endif /* #ifndef MP3DECODER_HH */
