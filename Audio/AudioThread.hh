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

#ifndef AUDIOTHREAD_HH
#define AUDIOTHREAD_HH

#include <stdio.h>
#include "mad.h"
#include "Thread.hh"
#include "StatusScreen.hh"

#define BUFFER_SIZE 50000

class StatusScreen;

class AudioThread : public Thread {
public:
    AudioThread(void);
    ~AudioThread(void);
    virtual void *ThreadMain(void *arg);
    void SetRequestedCommand(int Command);
    int GetActualCommand(void);
    enum mad_flow InputCallback(void *ptr, struct mad_stream *stream);
    enum mad_flow OutputCallback(void *ptr, struct mad_header const *header, struct mad_pcm *pcm);
    enum mad_flow ErrorCallback(void *ptr, struct mad_stream *stream, struct mad_frame *frame);
    
private:
    int GetRequestedCommand(void);
    void SetActualCommand(int Command);
    FILE *OpenFile(char *Filename);
    signed int ScaleSample(mad_fixed_t sample);
    unsigned char Buffer[BUFFER_SIZE];
    FILE *SongFP; /* File descriptor for song file */
    int AudioFD; /* File descriptor for audio output device */
    mad_timer_t CurrentTime;
    StatusScreen Status;
    struct mad_decoder MadDecoder;
    int CommandRequested;
    int CommandActual;
    int MetadataFrequency;
    int FirstRun;
    char *LocalBuffer;
};

inline signed int AudioThread::ScaleSample(mad_fixed_t sample) {
    /* This function is basically the audio_linear_round() function
       from the MAD distribution without the fancy clipping */
    /* Round */
    sample += (1L << (MAD_F_FRACBITS - 16));

    /* Clip */
    if (sample >= MAD_F_ONE)
        sample = MAD_F_ONE - 1;
    else if (sample < -MAD_F_ONE)
        sample = -MAD_F_ONE;

    /* Quantize */
    return sample >> (MAD_F_FRACBITS + 1 - 16);
}

#endif /* #ifndef AUDIOTHREAD_HH */
