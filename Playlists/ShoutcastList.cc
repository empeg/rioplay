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
#include <string.h>
#include <pthread.h>
#include "ShoutcastList.hh"
#include "Playlist.hh"
#include "Http.h"
#include "Screen.hh"
#include "Commands.h"
#include "DisplayThread.hh"
#include "MenuScreen.hh"
#include "KeyCodes.h"

extern DisplayThread Display;

ShoutcastList::ShoutcastList(void) {
    FILE *fp;
    char TempString[256];
    char *TempPtr;
    int CurrentStream = -1, CurrentUrl = 0;
    
    strcpy(StreamTitle, "Streaming Audio");
    
    Position = 1;
    UrlPosition = 1;
    StreamNames = NULL;
    StreamUrls = NULL;
    NumUrls = NULL;
    
    fp = fopen("/etc/streams.cfg", "r");
    
    while(fgets(TempString, 256, fp) != NULL) {
        if((TempPtr = strstr(TempString, "\r")) != NULL) {
            TempPtr[0] = '\0';
        }
        if((TempPtr = strstr(TempString, "\n")) != NULL) {
            TempPtr[0] = '\0';
        }
        if(strstr(TempString, "<title>") != NULL) {
            CurrentStream++;
            NumEntries++;
            
            /* Check to see if we need to expand the array */
            if((CurrentStream % 5) == 0) {
                StreamNames = (char **) realloc(StreamNames,
                        sizeof(char *) * (CurrentStream + 5));
                StreamUrls = (char ***) realloc(StreamUrls,
                        sizeof(char **) * (CurrentStream + 5));
                NumUrls = (int *) realloc(NumUrls,
                        sizeof(int) * (CurrentStream + 5));
            }
            /* Copy stream name */
            StreamNames[CurrentStream] = (char *) malloc(sizeof(char) *
                    (strlen(TempString + 7) + 1));
            strcpy(StreamNames[CurrentStream], TempString + 7);
            StreamUrls[CurrentStream] = NULL;
            NumUrls[CurrentStream] = 0;
            CurrentUrl = 0;
        }
        else {
            if((CurrentUrl % 5) == 0) {
                StreamUrls[CurrentStream] =
                        (char **) realloc(StreamUrls[CurrentStream],
                        sizeof(char *) * (CurrentUrl + 5));
            }
            StreamUrls[CurrentStream][CurrentUrl] =
                    (char *) malloc(sizeof(char) * (strlen(TempString) + 1));
            strcpy(StreamUrls[CurrentStream][CurrentUrl], TempString);
            if(StreamUrls[CurrentStream][CurrentUrl][strlen(TempString) - 1] == '\n') {
                StreamUrls[CurrentStream][CurrentUrl][strlen(TempString) - 1] = '\0';
            }
            CurrentUrl++;
            NumUrls[CurrentStream]++;
        }
    }
}

ShoutcastList::~ShoutcastList(void) {
}

Tag ShoutcastList::GetTag(int EntryNumber) {
    Tag ReturnVal;

    /* Set to zeros to be safe */
    bzero(&ReturnVal, sizeof(ReturnVal));
    
    /* Copy stream title into Title */
    strcpy(ReturnVal.Title, StreamTitle);
    
    /* Return the array of results */
    return ReturnVal;
}

char *ShoutcastList::GetFilename(char *Filename, int EntryNumber) {
    strcpy(Filename, StreamUrls[Position - 1][EntryNumber - 1]);
    strcpy(StreamTitle, StreamNames[Position - 1]);
    
    return Filename;
}

void ShoutcastList::SetMetadata(char *Metadata, int MetadataLength) {
    char *FirstQuote = NULL, *SecondQuote = NULL;
    int i;

    for(i = 0; i < MetadataLength; i++) {
        if(Metadata[i] == '\'') {
            if(FirstQuote == NULL) {
                FirstQuote = &(Metadata[i]);
            }
            else {
                SecondQuote = &(Metadata[i]);
                
                /* Cause the for loop to end */
                i = MetadataLength;
            }
        }
    }
    
    if((FirstQuote == NULL) || (SecondQuote == NULL)) {
        /* Didn't find two quotes */
        return;
    }
    
    /* Copy the title out of the metadata */
    if((SecondQuote - FirstQuote - 1) > 0) {
        memcpy(StreamTitle, FirstQuote + 1, SecondQuote - FirstQuote - 1);
        StreamTitle[SecondQuote - FirstQuote - 1] = '\0';
    }

    return;
}

int ShoutcastList::CommandHandler(unsigned int Keycode, MenuScreen *ActiveMenu) {
    static int CurrentMenu = 0;
    
    if((Keycode == PANEL_MENU) || (Keycode == REMOTE_MENU)) {
        Display.RemoveTopScreen(ActiveMenu);
        CurrentMenu = 0;
        return 0;
    }
    else if((Keycode == PANEL_WHEEL_CW) || (Keycode == REMOTE_DOWN) || (Keycode == REMOTE_DOWN_REPEAT)) {
        ActiveMenu->Advance();
        Display.Update(ActiveMenu);
        return 1;
    }
    else if((Keycode == PANEL_WHEEL_CCW) || (Keycode == REMOTE_UP) || (Keycode == REMOTE_UP_REPEAT)) {
        ActiveMenu->Reverse();
        Display.Update(ActiveMenu);
        return 1;
    }

    switch(CurrentMenu) {
        case 0: /* Rio Server Playlist root menu */
            ActiveMenu->ClearOptions();
            ActiveMenu->SetTitle("Select Stream");
            for(int i = 0; i < NumEntries; i++ ) {
                ActiveMenu->AddOption(StreamNames[i]);
            }
            CurrentMenu = 1;
            Display.Update(ActiveMenu);
            return 1;
            break;
            
        case 1:
            Position = ActiveMenu->GetSelection();
            UrlPosition = 1;
            Display.RemoveTopScreen(ActiveMenu);
            CurrentMenu = 0;
            return 2;
            break;
    }
    return 0;
}

void ShoutcastList::Advance(void) {
    UrlPosition++;
    if(UrlPosition > NumUrls[Position - 1]) {
        /* We've completed this sub-list, move on to the next sub-list */
        //Playlist::Advance();
        
        UrlPosition = 1;
    }
}        

void ShoutcastList::Reverse(void) {
    UrlPosition--;
    if(UrlPosition < 1) {
        /* We've completed this sub-list, move on to the previous sub-list */
        //Playlist::Reverse();

        UrlPosition = NumUrls[Position - 1];
    }
}

int ShoutcastList::GetPosition(void) {
    return UrlPosition;
}
