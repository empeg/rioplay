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
#include <sys/soundcard.h>
#include "RemoteThread.hh"
#include "DisplayThread.hh"
#include "AudioThread.hh"
#include "KeyCodes.h"
#include "RioServerList.hh"
#include "ShoutcastList.hh"
#include "MenuScreen.hh"
#include "Commands.h"
#include "Player.h"

extern DisplayThread Display;
extern AudioThread Audio;

RemoteThread::RemoteThread(void) {
    PList = NULL;
    TempPList = NULL;
    IrFD = -1;
    MixerFD = -1;
    Volume = 45;
    PlaylistMenuActive = 0;

    /* Open IR device for input */
    IrFD = open("/dev/ir", O_RDONLY);
    if(IrFD < 0) {
        printf("Remote: Error: Could not open IR device\n");
        pthread_exit(0);
    }
    
    /* Open mixer device for output */
    MixerFD = open("/dev/mixer", O_WRONLY);
    if(MixerFD < 0) {
        printf("Remote: Error: Could not open Mixer device\n");
        pthread_exit(0);
    }
    
    SetVolume();
}

RemoteThread::~RemoteThread(void) {
    if(PList) {
        delete PList;
    }
    if(TempPList) {
        delete TempPList;
    }
}

void *RemoteThread::ThreadMain(void *arg) {
    unsigned long KeyCode = 0;
    time_t timer = 0;

    while(1) {
        /* Handle the keypress */
        KeyCode = GetKeycode();
        switch(KeyCode) {
            case PANEL_POWER_UP:
                /* Check to see if power button was held for a long time */
                if((time(NULL) - timer) >= 5) {
                    printf("Remote: Received reboot command\n");
                    /* Turn off LCD */
                    Display.OnOff(0);
                    exit(1);
                }
                if((time(NULL) - timer) >= 2) {
                    printf("Remote: Received stop app command\n");
                    /* Turn off LCD */
                    Display.OnOff(0);
                    exit(0);
                }
                /* No break: we want to spill into the next case */
            case REMOTE_POWER:
                /* Stop audio playback */
                Audio.SetRequestedCommand(COMMAND_STOP);
                                
                /* Turn off LCD */
                Display.OnOff(0);

                /* Wait for power on again */
                for(unsigned long Key = 0; (Key != REMOTE_POWER) && 
                        (Key != PANEL_POWER_UP); Key = GetKeycode());
                timer = time(NULL);
                
                /* Resume audio */
                Audio.SetRequestedCommand(COMMAND_PLAY);

                /* Turn on LCD */
                Display.OnOff(1);
                break;
               
            case PANEL_POWER_DOWN:
                /* If user presses and holds the panel power button,
                   exit the player app */
                timer = time(NULL);
                 
            case REMOTE_PLAY:
            case PANEL_PLAY:
                /* Set command to play */
                if(Audio.GetActualCommand() == COMMAND_PLAY) {
                    Audio.SetRequestedCommand(COMMAND_PAUSE);
                }
                else {
                    Audio.SetRequestedCommand(COMMAND_PLAY);
                }
                break;

            case REMOTE_VOL_UP:
            case REMOTE_VOL_UP_REPEAT:
            case PANEL_VOL_UP:
                Volume += 2;
                SetVolume();
                printf("Remote: Volume is now %d\n", Volume);
                break;
                
            case REMOTE_VOL_DOWN:
            case REMOTE_VOL_DOWN_REPEAT:
            case PANEL_VOL_DOWN:
                Volume -= 2;
                SetVolume();
                printf("Remote: Volume is now %d\n", Volume);
                break;
                
            case REMOTE_STOP:
            case PANEL_STOP:
                /* Change requested command */
                Audio.SetRequestedCommand(COMMAND_STOP);
                break;
                
            case REMOTE_FORWARD:
            case PANEL_FORWARD:
                pthread_mutex_lock(&ClassMutex);
                if(PList) {
                    PList->Advance();
                }
                pthread_mutex_unlock(&ClassMutex);
                
                /* Change requested command */
                Audio.SetRequestedCommand(COMMAND_CHANGESONG);
                break;
                
            case REMOTE_REVERSE:
            case PANEL_REVERSE:
                pthread_mutex_lock(&ClassMutex);
                if(PList) {
                    PList->Reverse();
                }
                pthread_mutex_unlock(&ClassMutex);
                
                /* Change requested command */
                Audio.SetRequestedCommand(COMMAND_CHANGESONG);
                break;
                
            case REMOTE_MENU:
            case PANEL_MENU:
            case REMOTE_DOWN:
            case REMOTE_UP:
            case REMOTE_DOWN_REPEAT:
            case REMOTE_UP_REPEAT:
            case REMOTE_ENTER:
                if(PlaylistMenuActive == 0) {
                    MenuHandleKeypress(KeyCode);
                }
                if(PlaylistMenuActive != 0) {
                    int TempVal = TempPList->CommandHandler(KeyCode, &ActiveMenu);
                    if(TempVal == 0) {
                        PlaylistMenuActive = 0;
                        delete TempPList;
                        TempPList = NULL;
                    }
                    else if(TempVal == 2) {
                        PlaylistMenuActive = 0;
                        pthread_mutex_lock(&ClassMutex);
                        if(PList != NULL) {
                            delete PList;
                        }
                        PList = TempPList;
                        pthread_mutex_unlock(&ClassMutex);
                        TempPList = NULL;
                        Audio.SetRequestedCommand(COMMAND_CHANGESONG);
                    }
                }
                break;
                
            default:
                printf("Remote: Unhandled remote code 0x%08lx\n", KeyCode);
                break;
        }
    }
}

Playlist *RemoteThread::GetPlaylist(void) {
    Playlist *ReturnVal;
    
    pthread_mutex_lock(&ClassMutex);
    ReturnVal = PList;
    pthread_mutex_unlock(&ClassMutex);
    
    return ReturnVal;
}

void RemoteThread::SetVolume() {
    int VolOut;
    
    if(Volume > 100) {
        Volume = 100;
    }
    if(Volume < 0) {
        Volume = 0;
    }
    
    VolOut = Volume;
    
    VolOut = (VolOut & 0xff) | ((VolOut << 8) & 0xff00);
    if (ioctl(MixerFD, MIXER_WRITE(SOUND_MIXER_VOLUME), &VolOut) == -1) {
        perror("Mixer");
        return;
    }
}

unsigned long RemoteThread::GetKeycode(void) {
    unsigned long ReturnVal;
    
    /* Read KeyCode from IR device */
    if(read(IrFD, (char *) &ReturnVal, 4) < 0) {
        printf("Remote: Error reading from IR device\n");
        return 0;
    }
    
    return ReturnVal;
}

void RemoteThread::MenuHandleKeypress(unsigned long KeyCode) {
    static int CurrentMenu = 0;
    char TempString[32];
    
    if((KeyCode == PANEL_MENU) || (KeyCode == REMOTE_MENU)) {
        if(CurrentMenu == 0) {
            /* This menu is not currently active */
            ActiveMenu.ClearOptions();
            ActiveMenu.SetTitle("Audio Receiver");
            ActiveMenu.AddOption("Select Music");
            ActiveMenu.AddOption("About Receiver");
            //ActiveMenu.AddOption("Change Audio Settings");
            //ActiveMenu.AddOption("Change Display");

            /* Make the menu the active screen */
            Display.SetTopScreen(&ActiveMenu);
            Display.Update(&ActiveMenu);

            /* Update current menu */
            CurrentMenu = 1;
        }
        else {
            /* Menu pressed while a menu was displayed - exit menu system */
            Display.RemoveTopScreen(&ActiveMenu);
            CurrentMenu = 0;
        }
        return;
    }
    else if((KeyCode == PANEL_WHEEL_CW) || (KeyCode == REMOTE_DOWN)) {
        ActiveMenu.Advance();
        Display.Update(&ActiveMenu);
        return;
    }
    else if((KeyCode == PANEL_WHEEL_CCW) || (KeyCode == REMOTE_UP)) {
        ActiveMenu.Reverse();
        Display.Update(&ActiveMenu);
        return;
    }

    switch(CurrentMenu) {
        case 1: /* Audio Receiver menu */
            switch(ActiveMenu.GetSelection()) {
                case 1:
                    ActiveMenu.ClearOptions();
                    ActiveMenu.SetTitle("Select Music Source");
                    ActiveMenu.AddOption("Rio Server");
                    ActiveMenu.AddOption("Shoutcast");
                    CurrentMenu = 2;
                    Display.Update(&ActiveMenu);
                    break;
                    
                case 2: /* About Receiver menu */
                    ActiveMenu.ClearOptions();
                    ActiveMenu.SetTitle("About Receiver");
                    sprintf(TempString, "RioPlay version %s", PLAYER_VER);
                    ActiveMenu.AddOption(TempString);
                    ActiveMenu.AddOption("written by");
                    ActiveMenu.AddOption("David Flowerday");
                    CurrentMenu = 3;
                    Display.Update(&ActiveMenu);
                    break;
                    
                default:
                    /* Unimplemented menu */
                    break;
            }
            break;
            
        case 2:
            switch(ActiveMenu.GetSelection()) {
                case 1: /* Select Music selected */
                    pthread_mutex_lock(&ClassMutex);
                    TempPList = new RioServerList;
                    pthread_mutex_unlock(&ClassMutex);

                    PlaylistMenuActive = 1;
                    CurrentMenu = 0;
                    break;
                    
                case 2:
                    /* Create playlist and exit menu */
                    pthread_mutex_lock(&ClassMutex);
                    TempPList = new ShoutcastList;
                    pthread_mutex_unlock(&ClassMutex);
                    
                    PlaylistMenuActive = 1;
                    CurrentMenu = 0;
                    break;
                    
                default:
                    /* Unimplemented menu */
                    break;
            }
            break;
            
        case 3:
            Display.RemoveTopScreen(&ActiveMenu);
            CurrentMenu = 0;
            break;
    }
}
