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

#ifndef FLACDECODER_HH
#define FLACDECODER_HH

#include <stdio.h>
#include "Thread.hh"
#include "StatusScreen.hh"
#include "AudioOutputDevice.hh"
#include "InputSource.hh"
#include "Decoder.hh"
#include "libflac/stream_decoder.h"

#define BUFFER_SIZE 16384

class BufferClass;

class FlacDecoder : public Decoder {
public:
    FlacDecoder(int inInputFD, AudioOutputDevice *inAudioDev, InputSource *inPList);
    ~FlacDecoder(void);
    virtual void *ThreadMain(void *arg);
    
    /* These have to be public so the C jump functions can access them.
       They should not be used outside this class. */
    FLAC__StreamDecoderReadStatus InputCallback(const FLAC__StreamDecoder *decoder, FLAC__byte buffer[], unsigned *bytes, void *client_data);
    FLAC__StreamDecoderWriteStatus OutputCallback(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *client_data);
    void ErrorCallback(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data);
    void MetadataCallback(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data);
    
private:
    unsigned char *Buffer;
    int MetadataFrequency;
    int BufferSize;
    FLAC__StreamDecoder *FLACDecoder;
    
};

#endif /* #ifndef FLACDECODER_HH */
