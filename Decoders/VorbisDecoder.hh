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

#ifndef VORBISDECODER_HH
#define VORBISDECODER_HH

#include <stdio.h>
#include "Thread.hh"
#include "StatusScreen.hh"
#include "AudioOutputDevice.hh"
#include "InputSource.hh"
#include "Decoder.hh"
#include "libvorbisidec/ivorbiscodec.h"
#include "libvorbisidec/ivorbisfile.h"

#define BUFFER_SIZE 16384

class BufferClass;

class VorbisDecoder : public Decoder {
public:
    VorbisDecoder(int inInputFD, InputSource *inPList);
    ~VorbisDecoder(void);
    virtual void *ThreadMain(void *arg);
    size_t ReadCallback(void *ptr, size_t size, size_t nmemb, void *datasource);
    
private:
    unsigned char *Buffer;
    int BufferSize;

    OggVorbis_File vf;
    ov_callbacks Callbacks;
    char pcmout[4096];
};

#endif /* #ifndef VORBISDECODER_HH */
