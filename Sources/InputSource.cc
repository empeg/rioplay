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
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "InputSource.hh"
#include "Log.hh"
#include "Globals.hh"
#include "MemAlloc.hh"

extern int errno;

InputSource::InputSource(void) {
    NumEntries = 0;
    Position = 0;
    Dec = NULL;
    ServerConn = NULL;
}

InputSource::~InputSource(void) {
    Stop();
}

void InputSource::Stop(void) {
    if(Dec) {
        delete Dec;
        Dec = NULL;
    }
    if(ServerConn) {
        delete ServerConn;
        ServerConn = NULL;
    }
}

Tag InputSource::SetMetadata(char *Metadata, int MetadataLength) {
    /* Default behaviour is to ignore metadata.  Subclasses may
       override this if they have a need. */
    Tag ReturnTag;
    return ReturnTag;
}

HttpConnection *InputSource::OpenFile(char *Filename) {
    int fd;
    HttpConnection *Http = NULL;
    
    if(strstr(Filename, "http://") != NULL) {
        /* Found web file/stream */
        
        Http = new HttpConnection(Filename);
        
        /* Open a connection to the Rio HTTP Server */    
        fd = Http->Connect();
        if(fd < 0) {
            Log::GetInstance()->Post(LOG_FATAL, __FILE__, __LINE__,
                    "Couldn't connect to URL %s", Filename);
            return NULL;
        }
        
        /* Find end of HTTP Header */
        MetadataFrequency = Http->SkipHeader();
    
        /* Socket fd is now waiting with MP3 data on it */
    }
    else {
        /* Regular file */
        fd = open(Filename, O_RDONLY);
        if(fd < 0) {
            Log::GetInstance()->Post(LOG_FATAL, __FILE__, __LINE__,
                    "open() failed: %s", strerror(errno));
            return NULL;
        }
    }
    
    return Http;
}

void InputSource::DecoderFinished(void) {
    if(Dec) {
        delete Dec;
        Dec = NULL;
    }
    if(ServerConn != NULL) {
        delete ServerConn;
        ServerConn = NULL;
    }
    Globals::Playlist.DecoderFinished();
}
