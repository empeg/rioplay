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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include "RemoteThread.hh"
#include "KeyCodes.h"
#include "RioServerSource.hh"
#include "ShoutcastSource.hh"
#include "EmpegSource.hh"
#include "MenuScreen.hh"
#include "Player.h"
#include "Log.hh"
#include "Globals.hh"
#include "MemAlloc.hh"

extern int errno;

RemoteThread::RemoteThread(void) {
    PList = NULL;
    TempPList = NULL;
    IrFD = -1;
    CustomHandler = NULL;

    /* Open IR device for input */
    IrFD = open("/dev/ir", O_RDONLY);
    if(IrFD < 0) {
        Log::GetInstance()->Post(LOG_ERROR, __FILE__, __LINE__,
                "Could not open IR device");
        pthread_exit((void *) 1);
    }
    
}

RemoteThread::~RemoteThread(void) {
    close(IrFD);
}

void *RemoteThread::ThreadMain(void *arg) {
    unsigned long KeyCode = 0;
    time_t timer = 0;

    while(1) {
        /* Handle the keypress */
        KeyCode = GetKeycode();
        
        if(KeyCode == 0) {
	  if (Globals::hw_type != HWTYP_EMPEG) /* Hijack backlight no worky */
            /* Turn off backlight due to timeout */
            Globals::Display->Backlight(DISPLAY_BACKLIGHT_OFF);
        }
        else {
            /* Turn on backlight due to keypress */
            Globals::Display->Backlight(DISPLAY_BACKLIGHT_ON);
        
            switch(KeyCode) {
                case PANEL_POWER_UP:
                    /* Check to see if power button was held for a long time */
                    if((time(NULL) - timer) >= 5) {
                        Log::GetInstance()->Post(LOG_INFO, __FILE__, __LINE__,
                                "Received reboot command");
                        /* Stop playback (needed for appropriate
                           thread cleanup) */
                        /* Argument is true so that this command will
                           block until playlist is actually stopped */
                        Globals::Playlist.Stop(true);
                        
                        /* Turn off LCD */
                        Globals::Display->OnOff(0);
                        
                        fflush(stdout);
                        pthread_exit((void *) 1);
                    }
                    if((time(NULL) - timer) >= 2) {
                        Log::GetInstance()->Post(LOG_INFO, __FILE__, __LINE__,
                                "Received stop app command");
                        /* Stop playback (needed for appropriate
                           thread cleanup) */
                        /* Argument is true so that this command will
                           block until playlist is actually stopped */
                        Globals::Playlist.Stop(true);
                        
                        /* Turn off LCD */
                        Globals::Display->OnOff(0);
                        
                        fflush(stdout);
                        pthread_exit((void *) 0);
                    }
                    /* No break: we want to spill into the next case */
                case REMOTE_POWER:
                    /* Stop audio playback */
                    Globals::Playlist.Stop();

                    /* Turn off LCD */
                    Globals::Display->OnOff(0);

                    /* Log the power off event */
                    Log::GetInstance()->Post(LOG_INFO, __FILE__, __LINE__,
                            "RioPlay powering down");

                    /* Wait for power on again */
                    for(unsigned long Key = 0; (Key != REMOTE_POWER) && 
                            (Key != PANEL_POWER_UP); Key = GetKeycode());
                    timer = time(NULL);

                    /* Turn on LCD */
                    Globals::Display->OnOff(1);
                    break;

                case PANEL_POWER_DOWN:
                    /* If user presses and holds the panel power button,
                       exit the player app */
                    timer = time(NULL);
                    break;

                case REMOTE_PLAY:
                case PANEL_PLAY:
                    /* Check to see if a custom handler is installed 
                       that should be called */
                    if(CustomHandler != NULL) {
                        CustomHandler->Handle(KeyCode);
                    }
                    else {
                        Globals::Playlist.Play();
                    }
                    break;

                case REMOTE_VOL_UP:
                case REMOTE_VOL_UP_REPEAT:
                    Globals::AudioOut->SetVolume(
                            Globals::AudioOut->GetVolume() + 2);
                    break;

                case REMOTE_VOL_DOWN:
                case REMOTE_VOL_DOWN_REPEAT:
                    Globals::AudioOut->SetVolume(
                            Globals::AudioOut->GetVolume() - 2);
                    break;

                case REMOTE_STOP:
                case PANEL_STOP:
                    /* Change requested command */
                    Globals::Playlist.Stop();
                    break;

                case REMOTE_FORWARD:
                case PANEL_FORWARD:
                    /* Advance playlist */
                    Globals::Playlist.Forward();
                    break;

                case REMOTE_REVERSE:
                case PANEL_REVERSE:
                    /* Back up playlist */
                    Globals::Playlist.Reverse();
                    break;

                case REMOTE_RANDOM:
                case PANEL_RANDOM:
                    /* Randomize playlist */
                    Globals::Playlist.SetRandom();
                    Globals::Display->Update(Globals::Status);
                    break;
                    
                case REMOTE_LIST:
                    if(CustomHandler == NULL) {
                        Globals::Remote.InstallHandler(
                                Globals::Playlist.GetHandler());
                        CustomHandler->Handle(KeyCode);
                    }
                    else if(CustomHandler == Globals::Playlist.GetHandler()) {
                        CustomHandler->Handle(KeyCode);
                    }
                    break;
                    
                case REMOTE_MENU:
                case PANEL_MENU:
                case REMOTE_DOWN:
                case REMOTE_UP:
                case REMOTE_DOWN_REPEAT:
                case REMOTE_UP_REPEAT:
                case REMOTE_ENTER:
                case PANEL_WHEEL_CW:
                case PANEL_WHEEL_CCW:
                case PANEL_WHEEL_BUTTON:
                    if(CustomHandler == NULL) {
                        Handler.Handle(KeyCode);
                    }
                    /* Check to see if a custom handler is now installed 
                       that should be called */
                    if(CustomHandler != NULL) {
                        CustomHandler->Handle(KeyCode);
                    }
                    break;

                default:
                    Log::GetInstance()->Post(LOG_WARNING, __FILE__, __LINE__,
                        "Unhandled remote code 0x%x", KeyCode);
                    break;
            }
        }
    }
}

InputSource *RemoteThread::GetInputSource(void) {
    InputSource *ReturnVal;
    
    pthread_mutex_lock(&ClassMutex);
    ReturnVal = PList;
    pthread_mutex_unlock(&ClassMutex);
    
    return ReturnVal;
}

unsigned long RemoteThread::GetKeycode(void) {
    unsigned long ReturnVal;
    struct timeval Timeout;
    fd_set ReadFDs;
    
    Timeout.tv_sec = 30;
    Timeout.tv_usec = 0;
    FD_SET(IrFD, &ReadFDs);
    
    /* Wait for 30 seconds for a keypress */
    if(select(IrFD + 1, &ReadFDs, NULL, NULL, &Timeout) == 0) {
        return 0;
    }
    
    /* Read KeyCode from IR device */
    if(read(IrFD, (char *) &ReturnVal, 4) < 0) {
        Log::GetInstance()->Post(LOG_ERROR, __FILE__, __LINE__,
                "Error reading from IR device");
        return 0;
    }
    
    return ReturnVal;
}

void RemoteThread::InstallHandler(CommandHandler *inCustomHandler) {
    pthread_mutex_lock(&ClassMutex);
    CustomHandler = inCustomHandler;
    pthread_mutex_unlock(&ClassMutex);
}

void RemoteThread::RemoveHandler(void) {
    pthread_mutex_lock(&ClassMutex);
    CustomHandler = NULL;
    pthread_mutex_unlock(&ClassMutex);
}

RemoteCommandHandler::RemoteCommandHandler(void) {
    CurrentMenu = MENU_NONE;
}

RemoteCommandHandler::~RemoteCommandHandler(void) {
}

void RemoteCommandHandler::Handle(const unsigned long &KeyCode) {
    char TempString[32];
    
    if((KeyCode == PANEL_MENU) || (KeyCode == REMOTE_MENU)) {
        if(CurrentMenu == MENU_NONE) {
            /* This menu is not currently active */
            ActiveMenu.ClearOptions();
            ActiveMenu.SetTitle("Audio Receiver");
            ActiveMenu.AddOption("Select Music");
            ActiveMenu.AddOption("Clear Active Playlist");
            ActiveMenu.AddOption("About Receiver");
            //ActiveMenu.AddOption("Change Audio Settings");
            //ActiveMenu.AddOption("Change Display");

            /* Make the menu the active screen */
            Globals::Display->SetTopScreen(&ActiveMenu);
            Globals::Display->Update(&ActiveMenu);

            /* Update current menu */
            CurrentMenu = MENU_AUDIORECEIVER;
        }
        else {
            /* Menu pressed while a menu was displayed - exit menu system */
            Globals::Display->RemoveTopScreen(&ActiveMenu);
            CurrentMenu = MENU_NONE;
        }
        return;
    }
    else if(KeyCode == PANEL_WHEEL_CW) {
        if(CurrentMenu == MENU_NONE) {
            Globals::AudioOut->SetVolume(Globals::AudioOut->GetVolume() + 2);
        }
        else {
            ActiveMenu.Advance();
            Globals::Display->Update(&ActiveMenu);
        }
        return;
    }
    else if(KeyCode == PANEL_WHEEL_CCW) {
        if(CurrentMenu == MENU_NONE) {
            Globals::AudioOut->SetVolume(Globals::AudioOut->GetVolume() - 2);
        }
        else {
            ActiveMenu.Advance();
            Globals::Display->Update(&ActiveMenu);
        }
        return;
    }
    else if((KeyCode == REMOTE_DOWN) || (KeyCode == REMOTE_DOWN_REPEAT)) {
        ActiveMenu.Advance();
        Globals::Display->Update(&ActiveMenu);
        return;
    }
    else if((KeyCode == REMOTE_UP) || (KeyCode == REMOTE_UP_REPEAT)) {
        ActiveMenu.Reverse();
        Globals::Display->Update(&ActiveMenu);
        return;
    }

    switch(CurrentMenu) {
        case MENU_AUDIORECEIVER:
            switch(ActiveMenu.GetSelection()) {
                case 1:
                    ActiveMenu.ClearOptions();
                    ActiveMenu.SetTitle("Select Music Source");
                    ActiveMenu.AddOption("Rio Server");
                    ActiveMenu.AddOption("Shoutcast");
		    if (Globals::hw_type == HWTYP_EMPEG)
                        ActiveMenu.AddOption("Empeg");
                    CurrentMenu = MENU_MUSICSOURCE;
                    Globals::Display->Update(&ActiveMenu);
                    break;
                    
                case 2: /* Clear Active Playlist */
                    Globals::Playlist.Clear();
                    Globals::Display->RemoveTopScreen(&ActiveMenu);
                    CurrentMenu = MENU_NONE;
                    break;
                    
                case 3: /* About Receiver menu */
                    ActiveMenu.ClearOptions();
                    ActiveMenu.SetTitle("About Receiver");
                    sprintf(TempString, "RioPlay version %s", PLAYER_VER);
                    ActiveMenu.AddOption(TempString);
                    ActiveMenu.AddOption("written by");
                    ActiveMenu.AddOption("David Flowerday");
                    CurrentMenu = MENU_ABOUT;
                    Globals::Display->Update(&ActiveMenu);
                    break;
                    
                default:
                    /* Unimplemented menu */
                    break;
            }
            break;
            
        case MENU_MUSICSOURCE:
            switch(ActiveMenu.GetSelection()) {
                case 1: /* Rio Server selected */
                    Globals::Remote.InstallHandler(Globals::RioServer.GetHandler());
                    CurrentMenu = MENU_NONE;
                    break;
                    
                case 2: /* Shoutcast Server selected */
                    Globals::Remote.InstallHandler(Globals::Shoutcast.GetHandler());
                    CurrentMenu = MENU_NONE;
                    break;
                    
                case 3: /* Empeg selected */
		    if (Globals::hw_type == HWTYP_EMPEG)
		    {
			Globals::Remote.InstallHandler(Globals::Empeg->GetHandler());
                        CurrentMenu = MENU_NONE;
		    }
                    break;
                    
                default:
                    /* Unimplemented menu */
                    break;
            }
            break;
            
        case MENU_ABOUT:
            Globals::Display->RemoveTopScreen(&ActiveMenu);
            CurrentMenu = MENU_NONE;
            break;
    }
}
