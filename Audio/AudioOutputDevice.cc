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

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "AudioOutputDevice.hh"
#include "Log.hh"
#include "MemAlloc.hh"

AudioOutputDevice::AudioOutputDevice(void) {
    /* Open the audio device for output */
    AudioFD = open("/dev/audio", O_WRONLY);
    if (AudioFD < 0) {
        Log::GetInstance()->Post(LOG_FATAL, __FILE__, __LINE__,
                "Cannot open audio device");
        return;
    }
    
    /* Default the sample rate and bits/sample */
    SetSampleRate(44100);
    BitsPerSample = 16;
}

AudioOutputDevice::~AudioOutputDevice(void) {
    close(AudioFD);
}

void AudioOutputDevice::Play(const mad_fixed_t *Left, const mad_fixed_t *Right,
        unsigned int NumSamples) {
    //signed int LeftSample, RightSample;
    signed int LeftSample, RightSample;
    mad_fixed_t *Resampled[2] = {NULL, NULL};
    static int SamplePos = 0;
    int i;
    int NumNewSamples;
    const mad_fixed_t *Samples[2];

    if(InSampleRate != 44100) {
        /* Allocate space for resampled samples */
        Resampled[0] = (mad_fixed_t *) 
                __malloc(sizeof(mad_fixed_t) * NumSamples * 8);
        Resampled[1] = (mad_fixed_t *) 
                __malloc(sizeof(mad_fixed_t) * NumSamples * 8);
        NumNewSamples = ResampleBlock(NumSamples, Left, Resampled[0]);
        ResampleBlock(NumSamples, Right, Resampled[1]);
        Samples[0] = Resampled[0];
        Samples[1] = Resampled[1];
    }
    else {
        /* No need to resample */
        Samples[0] = Left;
        Samples[1] = Right;
        NumNewSamples = NumSamples;
    }
    
    for (i = 0; i < NumNewSamples; i++) {
        LeftSample = ScaleSample(Samples[0][i]);
        RightSample = ScaleSample(Samples[1][i]);
//        LeftSample = Samples[0][i];
//        RightSample = Samples[1][i];

        OutputBuffer[SamplePos++] = LeftSample & 0xff;
        OutputBuffer[SamplePos++] = (LeftSample >> 8) & 0xff;
        OutputBuffer[SamplePos++] = RightSample & 0xff;
        OutputBuffer[SamplePos++] = (RightSample >> 8) & 0xff;

        if (SamplePos == 4608) {
            /* Buffer full, write it to the audio device */
            write(AudioFD, OutputBuffer, 4608);

            /* Reset position in static sample buffer */
            SamplePos = 0;
        }
    }

    if(Resampled[0] != NULL) {
        __free(Resampled[0]);
        __free(Resampled[1]);
    }
}

unsigned int AudioOutputDevice::ResampleBlock(unsigned int NumSamples,
        mad_fixed_t const *OrigSamples, mad_fixed_t *NewSamples) {
    /* This function was copied from resample.c in the MAD distribution */
    /* This resampling algorithm is based on a linear interpolation, which is
       not at all the best sounding but is relatively fast and efficient.
      
       A better algorithm would be one that implements a bandlimited
       interpolation. */

    mad_fixed_t const *End, *Begin;

    if(State.ratio == MAD_F_ONE) {
        memcpy(NewSamples, OrigSamples, NumSamples * sizeof(mad_fixed_t));
        return NumSamples;
    }

    End = OrigSamples + NumSamples;
    Begin = NewSamples;

    if(State.step < 0) {
        State.step = mad_f_fracpart(-State.step);

        while(State.step < MAD_F_ONE) {
            if(State.step != 0) {
                *NewSamples++ = State.last +
                        mad_f_mul(*OrigSamples - State.last, State.step);
            }
            else {
                *NewSamples++ = State.last;
            }

            State.step += State.ratio;
            if (((State.step + 0x00000080L) & 0x0fffff00L) == 0) {
                State.step = (State.step + 0x00000080L) & ~0x0fffffffL;
            }
        }

        State.step -= MAD_F_ONE;
    }

    while(End - OrigSamples > 1 + mad_f_intpart(State.step)) {
        OrigSamples += mad_f_intpart(State.step);
        State.step = mad_f_fracpart(State.step);

        if(State.step != 0) {
            *NewSamples++ = *OrigSamples +
                    mad_f_mul(OrigSamples[1] - OrigSamples[0], State.step);
        }
        else {
            *NewSamples++ = *OrigSamples;
        }

        State.step += State.ratio;
        if (((State.step + 0x00000080L) & 0x0fffff00L) == 0) {
            State.step = (State.step + 0x00000080L) & ~0x0fffffffL;
        }
    }

    if(End - OrigSamples == 1 + mad_f_intpart(State.step)) {
        State.last = End[-1];
        State.step = -State.step;
    }
    else {
        State.step -= mad_f_fromint(End - OrigSamples);
    }

    return NewSamples - Begin;
}

void AudioOutputDevice::SetBitsPerSample(int inBitsPerSample) {
    BitsPerSample = inBitsPerSample;
}
