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
#include "DisplayThread.hh"
#include "AudioThread.hh"
#include "RemoteThread.hh"
#include "WebMain.h"

/* Be sure to have Display declared first */
DisplayThread Display;
RemoteThread Remote;
AudioThread Audio;

void *ThreadJump(void *arg) {
    Thread *T;
    
    T = (Thread *) arg;
    
    return T->ThreadMain(NULL);
}

int main() {
    pthread_t ThreadHandle[4]; /* We will have 4 threads */
    
    /* Print startup message */
    //printf("%c[2J", 27);
    printf("\n\n");
    printf("RioPlay version %s (%s) started\n", PLAYER_VER, PLAYER_DATE);
    printf("%s\n", PLAYER_COPYRIGHT);
    printf("Please see %s for more information\n\n", PLAYER_WEBADDR);
    
    /* Create Audio thread */
    pthread_create(&ThreadHandle[0], NULL, ThreadJump, &Audio);
    
    /* Create Display thread */
    pthread_create(&ThreadHandle[1], NULL, ThreadJump, &Display);
    
    /* Create Remote Control thread */
    pthread_create(&ThreadHandle[2], NULL, ThreadJump, &Remote);
    
    /* Create Web Interface thread */
    pthread_create(&ThreadHandle[3], NULL, WebMain, NULL);
    
    /* Wait for Audio thread to exit */
    pthread_join(ThreadHandle[0], NULL);
    
    printf("Main: Main thread exiting.\n");
    
    return 0;
}


