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

#include "Decoder.hh"
#include "BufferClass.hh"
#include "Globals.hh"
#include "MemAlloc.hh"

/* ThreadJump is defined in Start.cc */
extern void *ThreadJump(void *arg);

Decoder::Decoder(void) {
    /* Initialize member variables */
    Stop = false;
    Paused = false;
    SongFD = -1;
    PList = NULL;
    ExtBuffer = NULL;
    Reason = REASON_NONE;

    /* Initialize the status display */
    Globals::Display.SetBottomScreen(&Globals::Status);
}

Decoder::~Decoder(void) {
    /* Clean up member variables */
    if(ExtBuffer) {
        delete ExtBuffer;
    }
}

void Decoder::SetMetadataFrequency(int Freq) {
}

void Decoder::SetPause(int pause) {
    if(pause == 0)
    {
    	    Paused = false;
	    pthread_cond_signal(&ClassCondition);
    }
    else if (pause == 1)
	    Paused = true;
}
