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

#ifndef REMOTETHREAD_HH
#define REMOTETHREAD_HH

#include "Thread.hh"
#include "MenuScreen.hh"

class InputSource;

class RemoteThread : public Thread {
public:
    RemoteThread(void);
    ~RemoteThread(void);
    virtual void *ThreadMain(void *arg);
    InputSource *GetInputSource(void);
    
private:
    void SetVolume();
    unsigned long GetKeycode(void);
    void MenuHandleKeypress(unsigned long KeyCode);
    InputSource *PList, *TempPList;
    int IrFD;
    int MixerFD;
    int Volume;
    MenuScreen ActiveMenu;
    int InputSourceMenuActive;
};

#endif /* #ifndef REMOTETHREAD_HH */
