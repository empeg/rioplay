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

#ifndef LLMADDECODER_HH
#define LLMADDECODER_HH

#include <stdio.h>
#include "libmad/mad.h"
#include "Thread.hh"
#include "StatusScreen.hh"
#include "AudioOutputDevice.hh"
#include "InputSource.hh"
#include "Decoder.hh"

class LLMadDecoder : public Decoder {
public:
    LLMadDecoder(int inInputFD, InputSource *inPList);
    ~LLMadDecoder(void);
    virtual void *ThreadMain(void *arg);
    
private:
    mad_timer_t CurrentTime;
    struct mad_stream Stream;
    struct mad_frame Frame;
    struct mad_synth Synth;
};

#endif /* #ifndef LLMADDECODER_HH */
