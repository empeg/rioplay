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

#ifndef RIORECEIVERAUDIO_HH
#define RIORECEIVERAUDIO_HH

#include "AudioOutputDevice.hh"
#include "libmad/mad.h"

#define RIO_RECEIVER_AUDIO_BLOCK_SIZE 4608

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

class RioReceiverAudio : public AudioOutputDevice {
public:
    RioReceiverAudio(void);
    ~RioReceiverAudio(void);
    void SetSampleRate(unsigned int newRate);
    void Play(const mad_fixed_t *Left, const mad_fixed_t *Right,
            unsigned int NumSamples);
    void Play(const char *Data, unsigned int Size);
    void Flush(void);
    void SetVolume(int inVolume);
    
protected:
    signed int ScaleSample(mad_fixed_t Sample);
    int ResampleRateIndex(unsigned int Rate);
    int ResampleInit();
    unsigned int ResampleBlock(unsigned int NumSamples,
            mad_fixed_t const *OrigSamples, mad_fixed_t *NewSamples);
    int AudioFD, MixerFD;
    ResampleState State;
    char OutputBuffer[RIO_RECEIVER_AUDIO_BLOCK_SIZE + 16];
    int SamplePos;
};

inline void RioReceiverAudio::SetSampleRate(unsigned int newRate) {
    if(newRate != InSampleRate) {
        InSampleRate = newRate;
        ResampleInit();
    }
}

inline signed int RioReceiverAudio::ScaleSample(mad_fixed_t Sample) {
    /* This function is basically the audio_linear_round() function
       from the MAD distribution without the fancy clipping */
    if(BitsPerSample > 16) {
        /* Round */
        Sample += (1L << (BitsPerSample - 16));

        /* Clip */
        if(Sample >= MAD_F_ONE) {
            Sample = MAD_F_ONE - 1;
        }
        else if(Sample < -MAD_F_ONE) {
            Sample = -MAD_F_ONE;
        }

        /* Quantize */
        return Sample >> (BitsPerSample + 1 - 16);
    }
    else {
        return Sample;
    }
}

inline int RioReceiverAudio::ResampleRateIndex(unsigned int Rate) {
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

inline int RioReceiverAudio::ResampleInit(void) {
    int RateIndex;

    RateIndex = ResampleRateIndex(InSampleRate);

    if(RateIndex == -1) {
      return -1;
    }

    State.ratio = ResampleTable[RateIndex];

    State.step = 0;
    State.last = 0;

    return 0;
}

#endif
