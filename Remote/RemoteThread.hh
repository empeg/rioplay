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
#include "CommandHandler.hh"

class InputSource;

class RemoteCommandHandler : public CommandHandler {
public:
    RemoteCommandHandler(void);
    virtual ~RemoteCommandHandler(void);
    void Handle(const unsigned long &Keycode);
    int GetCurrentMenu() { return CurrentMenu; };
private:
    MenuScreen ActiveMenu;
    enum MenuTypes {
        MENU_NONE = 0,
        MENU_AUDIORECEIVER,
        MENU_MUSICSOURCE,
        MENU_ABOUT
    };
        
    int CurrentMenu;
};

class RemoteThread : public Thread {
public:
    RemoteThread(void);
    ~RemoteThread(void);
    virtual void *ThreadMain(void *arg);
    InputSource *GetInputSource(void);
    void InstallHandler(CommandHandler *inCustomHandler);
    void RemoveHandler(void);
    
private:
    unsigned long GetKeycode(void);
    void MenuHandleKeypress(unsigned long KeyCode);
    InputSource *PList, *TempPList;
    int IrFD;
    MenuScreen ActiveMenu;
    int InputSourceMenuActive;
    RemoteCommandHandler Handler;
    CommandHandler *CustomHandler;
};

#endif /* #ifndef REMOTETHREAD_HH */
