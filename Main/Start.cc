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
#include "EmpegAudio.hh"
#include "MemAlloc.hh"

/* MemAlloc class needs to be declared first so it is destroyed last */
#ifdef ALLOC_DEBUG
MemAlloc DummyMemAlloc;
#endif

/* Be sure to have Display declared before the other thread types */
DisplayThread *Globals::Display = NULL;
RemoteThread Globals::Remote;
WebThread Globals::Web;
StatusScreen *Globals::Status = NULL;
PlaylistClass Globals::Playlist;
AudioOutputDevice *Globals::AudioOut = NULL;
RioServerSource Globals::RioServer;
ShoutcastSource Globals::Shoutcast;
EmpegSource *Globals::Empeg = NULL;
int Globals::hw_type;
Log DummyLog;

#define IDFILE "/proc/empeg_id"

int main() {
    int ReturnVal;
    char    s[64];
    char    hwname[32];
    FILE    *fp;

    
    /* Print startup message */
    printf("\n\n");
    printf("RioPlay version %s (%s) started\n", PLAYER_VER, PLAYER_DATE);
    printf("%s\n", PLAYER_COPYRIGHT);
    printf("Please see %s for more information\n\n", PLAYER_WEBADDR);


    /* Determine what hardware platform we're running on. */

    Globals::hw_type = HWTYP_RIORCV;	/* Default to receiver */
    strcpy(hwname, HWNM_RIORCV);
    
    fp=fopen(IDFILE, "rb");
    while (fgets(s,sizeof(s), fp) != NULL) {
            s[strlen(s)-1]='\0';
	    if (strstr(s, "drives:") != NULL)
	    {   /* only the empeg has the "drives" parameter */
	    	Globals::hw_type = HWTYP_EMPEG;
		strcpy(hwname, HWNM_EMPEG);
		break;
	    }
    }
    fclose(fp);
    	
    printf("Hardware Platform: %s\n\n", hwname); 

    Globals::Display = new DisplayThread; 
    Globals::Status = new StatusScreen; 

    /* Do hardware dependent setup here */
    if (Globals::hw_type == HWTYP_RIORCV)
	Globals::AudioOut = new RioReceiverAudio;
    else if (Globals::hw_type == HWTYP_EMPEG)
    {
    	    Globals::AudioOut = new EmpegAudio;
	    Globals::Empeg = new EmpegSource;
    }
    
    /* Create Playlist thread */
    Globals::Playlist.Start();
        
    /* Create Display thread */
    Globals::Display->Start();
    
    /* Create Remote Control thread */
    Globals::Remote.Start();
    
    /* Create Web Interface thread */
    Globals::Web.Start();
    
    /* Wait for Remote Control thread to exit */
    pthread_join(*Globals::Remote.GetHandle(), (void **) &ReturnVal);
    
    printf("Main: Main thread exiting (exit status %d)\n", ReturnVal);

    if (Globals::Empeg) delete Globals::Empeg;
    if (Globals::AudioOut) delete Globals::AudioOut;
    //should we delete these too for fear of memory leaks?
    //if (Globals::Status) delete Globals::Status;
    //if (Globals::Display) delete Globals::Display;
    
    return ReturnVal;
}


