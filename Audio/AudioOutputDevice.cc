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

#include "AudioOutputDevice.hh"
#include "MemAlloc.hh"

AudioOutputDevice::AudioOutputDevice(void) {
    /* Initialize member variables */
    Volume = 0;
    InSampleRate = 44100;
    BitsPerSample = 16;
}

AudioOutputDevice::~AudioOutputDevice(void) {
}
