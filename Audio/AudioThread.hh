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
#include "Http.hh"
#include "StatusScreen.hh"

#define BUFFER_SIZE 16384

struct ResampleState {
    mad_fixed_t ratio;
    mad_fixed_t step;
    mad_fixed_t last;
};

static mad_fixed_t const ResampleTable[9] = {
    /* 48000 */ MAD_F(0x116a3b36) /* 1.088435374 */,
    /* 44100 */ MAD_F(0x10000000) /* 1.000000000 */,
    /* 32000 */ MAD_F(0x0b9c2779) /* 0.725623583 */,
    /* 24000 */ MAD_F(0x08b51d9b) /* 0.544217687 */,
    /* 22050 */ MAD_F(0x08000000) /* 0.500000000 */,
    /* 16000 */ MAD_F(0x05ce13bd) /* 0.362811791 */,
    /* 12000 */ MAD_F(0x045a8ecd) /* 0.272108844 */,
    /* 11025 */ MAD_F(0x04000000) /* 0.250000000 */,
    /*  8000 */ MAD_F(0x02e709de) /* 0.181405896 */
};

class BufferClass;

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
    HttpConnection *OpenFile(char *Filename);
    signed int ScaleSample(mad_fixed_t Sample);
    int ResampleRateIndex(unsigned int Rate);
    int ResampleInit(unsigned int OrigRate);
    unsigned int ResampleBlock(unsigned int NumSamples,
            mad_fixed_t const *OrigSamples, mad_fixed_t *NewSamples);
    unsigned char *Buffer;
    int SongFD; /* File descriptor for song file */
    int AudioFD; /* File descriptor for audio output device */
    mad_timer_t CurrentTime;
    StatusScreen Status;
    struct mad_decoder MadDecoder;
    int CommandRequested;
    int CommandActual;
    int MetadataFrequency;
    int FirstRun;
    char *LocalBuffer;
    int BufferSize;
    BufferClass *ExtBuffer;
    ResampleState State;
    unsigned int Mp3SampleRate;
    
};

inline signed int AudioThread::ScaleSample(mad_fixed_t Sample) {
    /* This function is basically the audio_linear_round() function
       from the MAD distribution without the fancy clipping */
    /* Round */
    Sample += (1L << (MAD_F_FRACBITS - 16));

    /* Clip */
    if(Sample >= MAD_F_ONE) {
        Sample = MAD_F_ONE - 1;
    }
    else if(Sample < -MAD_F_ONE) {
        Sample = -MAD_F_ONE;
    }

    /* Quantize */
    return Sample >> (MAD_F_FRACBITS + 1 - 16);
}

inline int AudioThread::ResampleRateIndex(unsigned int Rate) {
  switch(Rate) {
      case 48000: return 0;
      case 44100: return 1;
      case 32000: return 2;
      case 24000: return 3;
      case 22050: return 4;
      case 16000: return 5;
      case 12000: return 6;
      case 11025: return 7;
      case  8000: return 8;
  }

  return -1;
}    

inline int AudioThread::ResampleInit(unsigned int OrigRate) {
    int RateIndex;

    RateIndex = ResampleRateIndex(OrigRate);

    if(RateIndex == -1) {
      return -1;
    }

    State.ratio = ResampleTable[RateIndex];

    State.step = 0;
    State.last = 0;

    return 0;
}

#endif /* #ifndef AUDIOTHREAD_HH */
