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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "DisplayThread.hh"
#include "Log.hh"

DisplayThread::DisplayThread(void) {
    /* Initialize class variables */
    TopScreenPtr = NULL;
    BottomScreenPtr = NULL;
    
    /* Open display device */
    DisplayFD = open("/dev/display", O_RDWR);
    if(DisplayFD < 0) {
        Log::GetInstance()->Post(LOG_ERROR, __FILE__, __LINE__,
                "Display: Could not open display");
        return;
    }
    Display = (char *) mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, DisplayFD, 0);
    
    /* Clear the display */
    memset(Display, 0, 4096);
    Push();
    
    /* Power on the display */
    OnOff(1);
    
    /* Show the splashscreen */
    BottomScreenPtr = &Logo;
    Logo.Update(Display);
    Push();
}

DisplayThread::~DisplayThread(void) {
}

void *DisplayThread::ThreadMain(void *arg) {
    while(1) {
        /* Lock and wait for signal that display needs updating */
        pthread_mutex_lock(&ClassMutex);
        pthread_cond_wait(&ClassCondition, &ClassMutex);

        if(TopScreenPtr != NULL) {
            /* If the top screen ptr is not null, then there is a top
               screen to display (top as in higher priority screen
               than the bottom [for instance a menu screen should
               "cover up" the audio player status screen]) */
            if(TopChanged == true) {
                Clear();
                TopChanged = false;
            }
            TopScreenPtr->Update(Display);
        }
        else if(BottomScreenPtr != NULL) {
            /* No top screen, but there is a bottom screen so show that */
            if(BottomChanged == true) {
                Clear();
                BottomChanged = false;
            }
            BottomScreenPtr->Update(Display);
        }
        Push();
        
        /* Unlock display */
        pthread_mutex_unlock(&ClassMutex);
    }
}
    
void DisplayThread::SetTopScreen(Screen *ScreenPtr) {
    pthread_mutex_lock(&ClassMutex);
    TopScreenPtr = ScreenPtr;
    TopChanged = true;
    pthread_mutex_unlock(&ClassMutex);
}

void DisplayThread::SetBottomScreen(Screen *ScreenPtr) {
    pthread_mutex_lock(&ClassMutex);
    BottomScreenPtr = ScreenPtr;
    BottomChanged = true;
    pthread_mutex_unlock(&ClassMutex);
}

void DisplayThread::RemoveTopScreen(Screen *ScreenPtr) {
    Screen *TempScreenPtr = NULL;
    
    pthread_mutex_lock(&ClassMutex);
    if(ScreenPtr == TopScreenPtr) {
        TopScreenPtr = NULL;
    }
    if(BottomScreenPtr != NULL) {
        /* Call update to redraw the bottom screen */
        TempScreenPtr = BottomScreenPtr;
        BottomChanged = true;
    }
    pthread_mutex_unlock(&ClassMutex);
    
    /* Redraw the bottom screen if it's available */
    if(TempScreenPtr != NULL) {
        Update(TempScreenPtr);
    }
}

void DisplayThread::RemoveBottomScreen(Screen *ScreenPtr) {
    pthread_mutex_lock(&ClassMutex);
    if(ScreenPtr == BottomScreenPtr) {
        BottomScreenPtr = NULL;
    }
    pthread_mutex_unlock(&ClassMutex);
}

void DisplayThread::Update(Screen *ScreenPtr) {
    pthread_mutex_lock(&ClassMutex);
    if(ScreenPtr == TopScreenPtr) {
        /* Requested update of top screen display */
        pthread_cond_signal(&ClassCondition);
    }
    else if((TopScreenPtr == NULL) && (ScreenPtr == BottomScreenPtr)) {
        /* Requested update of bottom screen, and no top screen is present */
        pthread_cond_signal(&ClassCondition);
    }
    else {
        /* Reqested update of screen that is not currently visible,
           so we'll ignore it */
    }
    pthread_mutex_unlock(&ClassMutex);
}
