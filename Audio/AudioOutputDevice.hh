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

#ifndef AUDIOOUTPUTDEVICE_HH
#define AUDIOOUTPUTDEVICE_HH

#include "libmad/mad.h"

class AudioOutputDevice {
public:
    AudioOutputDevice(void);
    virtual ~AudioOutputDevice(void);
    virtual void SetSampleRate(unsigned int newRate);
    virtual void SetBitsPerSample(int inBitsPerSample);
    virtual void Play(const mad_fixed_t *Left, const mad_fixed_t *Right,
            unsigned int NumSamples) = 0;
    virtual void Play(const char *Data, unsigned int Size) = 0;
    virtual void Flush(void) = 0;
    virtual void SetVolume(int inVolume) = 0;
    virtual int GetVolume(void);
    
protected:
    unsigned int InSampleRate;
    int BitsPerSample;
    int Volume;
};

inline void AudioOutputDevice::SetSampleRate(unsigned int newRate) {
    if(newRate != InSampleRate) {
        InSampleRate = newRate;
    }
}

inline void AudioOutputDevice::SetBitsPerSample(int inBitsPerSample) {
    BitsPerSample = inBitsPerSample;
}

inline int AudioOutputDevice::GetVolume(void) {
    return Volume;
}

#endif
