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

#ifndef INPUTSOURCE_HH
#define INPUTSOURCE_HH

#include <pthread.h>
#include "Tag.h"
#include "Decoder.hh"
#include "Http.hh"

class MenuScreen;
class CommandHandler;

class InputSource {
public:
    InputSource(void);
    virtual ~InputSource(void);
    virtual void Stop(void);
    virtual void Play(unsigned int ID) = 0;
    virtual Tag GetTag(int EntryNumber) = 0;
    virtual Tag SetMetadata(char *Metadata, int MetadataLength);
    void DecoderFinished(void);

protected:
    HttpConnection *OpenFile(char *Filename);
    int NumEntries;
    int Position;
    Decoder *Dec;
    pthread_mutex_t ClassMutex;
    pthread_cond_t ClassCondition;
    HttpConnection *ServerConn;
    int MetadataFrequency;
    int LocalFD;        
private:
};
    
#endif /* #ifndef INPUTSOURCE_HH */
