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

#ifndef DISPLAY_HH
#define DISPLAY_HH

#include <unistd.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <string.h>
#include "Thread.hh"
#include "Screen.hh"
#include "LogoScreen.hh"

#define DISPLAY_DELAY 20000
#define DISPLAY_BACKLIGHT_ON  4
#define DISPLAY_BACKLIGHT_OFF 0

class DisplayThread : public Thread {
public:
    DisplayThread(void);
    ~DisplayThread(void);
    virtual void *ThreadMain(void *arg);
    void SetTopScreen(Screen *ScreenPtr);
    void SetBottomScreen(Screen *ScreenPtr);
    void RemoveTopScreen(Screen *ScreenPtr);
    void RemoveBottomScreen(Screen *ScreenPtr);
    void Update(Screen *ScreenPtr);
    void OnOff(int OnOff);
    void Backlight(int State);
    void ShowHourglass(void);
    
private:
    void Push(void);
    void Clear(void);
    void DrawPixel(bool Set, int x, int y);
    int DisplayFD;
    char *Display;
    Screen *TopScreenPtr;
    Screen *BottomScreenPtr;
    bool TopChanged;
    bool BottomChanged;
    LogoScreen Logo;
    int BacklightState;
};

inline void DisplayThread::Clear(void) {
    bzero(Display, 4096);
}

inline void DisplayThread::Push(void) {
    ioctl((DisplayFD), _IO('d', 0));
    usleep(DISPLAY_DELAY);
}

inline void DisplayThread::OnOff(int OnOff) {
    /* Power on/off the display */
    pthread_mutex_lock(&ClassMutex);
    ioctl((DisplayFD), _IOW('d', 1, int), (OnOff));
    usleep(DISPLAY_DELAY);
    pthread_mutex_unlock(&ClassMutex);
}

inline void DisplayThread::Backlight(int State) {
    /* Turn on/off the backlight */
    pthread_mutex_lock(&ClassMutex);
    if(BacklightState != State) {
        ioctl((DisplayFD), _IOW('d', 11, int), &State);
        BacklightState = State;
        usleep(DISPLAY_DELAY);
    }
    pthread_mutex_unlock(&ClassMutex);
}

#endif /* #ifndef DISPLAY_HH */
