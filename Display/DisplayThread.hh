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
    
private:
    void Push(void);
    void Clear(void);
    int DisplayFD;
    char *Display;
    Screen *TopScreenPtr;
    Screen *BottomScreenPtr;
    bool TopChanged;
    bool BottomChanged;
    LogoScreen Logo;
};

inline void DisplayThread::Clear(void) {
    bzero(Display, 4096);
}

inline void DisplayThread::Push(void) {
    ioctl((DisplayFD), _IO('d', 0));
    usleep(200000);
}

inline void DisplayThread::OnOff(int OnOff) {
    /* Power on/off the display */
    ioctl((DisplayFD), _IOW('d', 1, int), (OnOff));
    usleep(200000);
    
    /* Redisplay the correct screen if one is present */
//    if(OnOff != 0) {
//        Update(BottomScreenPtr);
//        Update(TopScreenPtr);
//   }
}

#endif /* #ifndef DISPLAY_HH */
