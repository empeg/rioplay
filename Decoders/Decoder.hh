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

#ifndef DECODER_HH
#define DECODER_HH

#include "Thread.hh"
#include "StatusScreen.hh"

#define REASON_NONE           0
#define REASON_STOP_REQUESTED 1
#define REASON_READ_FAILED    2

class InputSource;
class BufferClass;
class AudioOutputDevice;

class Decoder : public Thread {
public:
    Decoder(void);
    virtual ~Decoder(void);
    virtual void *ThreadMain(void *arg) = 0;
    virtual void SetMetadataFrequency(int Freq);

protected:
    bool Paused, Stop;
    int SongFD;
    InputSource *PList;
    BufferClass *ExtBuffer;
    int Reason;
};

#endif /* #ifndef DECODER_HH */
