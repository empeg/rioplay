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
#include <pthread.h>
#include <string.h>
#include "Player.h"
#include "Log.hh"
#include "Globals.hh"
#include "RioReceiverAudio.hh"
#include "MemAlloc.hh"

/* MemAlloc class needs to be declared first so it is destroyed last */
#ifdef ALLOC_DEBUG
MemAlloc DummyMemAlloc;
#endif

/* Be sure to have Display declared before the other thread types */
DisplayThread Globals::Display;
RemoteThread Globals::Remote;
WebThread Globals::Web;
StatusScreen Globals::Status;
PlaylistClass Globals::Playlist;
AudioOutputDevice *Globals::AudioOut = NULL;
RioServerSource Globals::RioServer;
ShoutcastSource Globals::Shoutcast;
Log DummyLog;

int main() {
    int ReturnVal;
    
    /* Print startup message */
    printf("\n\n");
    printf("RioPlay version %s (%s) started\n", PLAYER_VER, PLAYER_DATE);
    printf("%s\n", PLAYER_COPYRIGHT);
    printf("Please see %s for more information\n\n", PLAYER_WEBADDR);

    /* Do hardware dependent setup here */
    Globals::AudioOut = new RioReceiverAudio;
    
    /* Create Playlist thread */
    Globals::Playlist.Start();
        
    /* Create Display thread */
    Globals::Display.Start();
    
    /* Create Remote Control thread */
    Globals::Remote.Start();
    
    /* Create Web Interface thread */
    Globals::Web.Start();
    
    /* Wait for Remote Control thread to exit */
    pthread_join(*Globals::Remote.GetHandle(), (void **) &ReturnVal);
    
    printf("Main: Main thread exiting (exit status %d)\n", ReturnVal);
    return ReturnVal;
}


