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
#include <errno.h>
#include "ShoutcastSource.hh"
#include "Screen.hh"
#include "MenuScreen.hh"
#include "KeyCodes.h"
#include "Log.hh"
#include "Mp3Decoder.hh"
#include "FlacDecoder.hh"
#include "Globals.hh"
#include "CommandHandler.hh"
#include "MemAlloc.hh"

extern int errno;

ShoutcastSource::ShoutcastSource(void) {
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
    Handler = new ShoutcastCommandHandler(this);

    if((fp = fopen("/etc/streams.cfg", "r"))==NULL){
      Log::GetInstance()->Post(LOG_ERROR, __FILE__, __LINE__,
			     "streams.cfg is missing");
      return;
    }
    
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
                StreamNames = (char **) __realloc(StreamNames,
                        sizeof(char *) * (CurrentStream + 5));
                StreamUrls = (char ***) __realloc(StreamUrls,
                        sizeof(char **) * (CurrentStream + 5));
                NumUrls = (int *) __realloc(NumUrls,
                        sizeof(int) * (CurrentStream + 5));
            }
            /* Copy stream name */
            StreamNames[CurrentStream] = (char *) __malloc(sizeof(char) *
                    (strlen(TempString + 7) + 1));
            strcpy(StreamNames[CurrentStream], TempString + 7);
            StreamUrls[CurrentStream] = NULL;
            NumUrls[CurrentStream] = 0;
            CurrentUrl = 0;
        }
        else {
            if((CurrentUrl % 5) == 0) {
                StreamUrls[CurrentStream] =
                        (char **) __realloc(StreamUrls[CurrentStream],
                        sizeof(char *) * (CurrentUrl + 5));
            }
            StreamUrls[CurrentStream][CurrentUrl] =
                    (char *) __malloc(sizeof(char) * (strlen(TempString) + 1));
            strcpy(StreamUrls[CurrentStream][CurrentUrl], TempString);
            if(StreamUrls[CurrentStream][CurrentUrl][strlen(TempString) - 1] == '\n') {
                StreamUrls[CurrentStream][CurrentUrl][strlen(TempString) - 1] = '\0';
            }
            CurrentUrl++;
            NumUrls[CurrentStream]++;
        }
    }
}

ShoutcastSource::~ShoutcastSource(void) {
    /* Stop playing and clean up decoder */
    Stop();
    
    /* Clean up other member variables */
    for(int i = 0; i < NumEntries; i++) {
        __free(StreamNames[i]);
        for(int j = 0; j < NumUrls[i]; j++) {
            __free(StreamUrls[i][j]);
        }
        __free(StreamUrls[i]);
    }
    __free(StreamNames);
    __free(StreamUrls);
    __free(NumUrls);
}

Tag ShoutcastSource::GetTag(int EntryNumber) {
    Tag ReturnVal;

    /* Set to zeros to be safe */
    bzero(&ReturnVal, sizeof(ReturnVal));
    
    /* Copy stream title into Title */
    strcpy(ReturnVal.Title, StreamTitle);
    
    /* Return the array of results */
    return ReturnVal;
}

Tag ShoutcastSource::SetMetadata(char *Metadata, int MetadataLength) {
    char *FirstQuote = NULL, *SecondQuote = NULL;
    int i;
    Tag ReturnVal;

    /* Set to zeros to be safe */
    bzero(&ReturnVal, sizeof(ReturnVal));
    
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
        return ReturnVal;
    }
    
    /* Copy the title out of the metadata */
    if((SecondQuote - FirstQuote - 1) > 0) {
        memcpy(StreamTitle, FirstQuote + 1, SecondQuote - FirstQuote - 1);
        StreamTitle[SecondQuote - FirstQuote - 1] = '\0';
    }

    /* Copy stream title into Title */
    strcpy(ReturnVal.Title, StreamTitle);
    
    return ReturnVal;
}

ShoutcastCommandHandler::ShoutcastCommandHandler(ShoutcastSource *inShoutcast) {
    Shoutcast = inShoutcast;
}

ShoutcastCommandHandler::~ShoutcastCommandHandler(void) {
}

void ShoutcastCommandHandler::Handle(const unsigned long &Keycode) {
    if((Keycode == PANEL_MENU) || (Keycode == REMOTE_MENU)) {
        Globals::Display->RemoveTopScreen(&Menu);
        Globals::Remote.RemoveHandler();
        CurrentMenu = MENU_NONE;
        return;
    }
    else if((Keycode == PANEL_WHEEL_CW) || (Keycode == REMOTE_DOWN) ||
        (Keycode == REMOTE_DOWN_REPEAT)) {
        Menu.Advance();
        Globals::Display->Update(&Menu);
        return;
    }
    else if((Keycode == PANEL_WHEEL_CCW) || (Keycode == REMOTE_UP) ||
        (Keycode == REMOTE_UP_REPEAT)) {
        Menu.Reverse();
        Globals::Display->Update(&Menu);
        return;
    }

    switch(CurrentMenu) {
        case MENU_NONE: /* Shoutcast root menu */
            Menu.ClearOptions();
            Menu.SetTitle("Select Stream");
            for(int i = 0; i < Shoutcast->NumEntries; i++ ) {
                Menu.AddOption(Shoutcast->StreamNames[i]);
            }
            CurrentMenu = MENU_STREAM;
            Globals::Display->SetTopScreen(&Menu);
            Globals::Display->Update(&Menu);
            return;
            break;
            
        case MENU_STREAM: /* Stream selection menu */
            Shoutcast->UrlPosition = 1;
            CurrentMenu = MENU_NONE;
            
            /* Clear the playlist if the user used the "Play" button
               (leave the playlist intact if "Enter" was used) */
            if((Keycode == REMOTE_PLAY) || (Keycode == PANEL_PLAY)) {
                Globals::Playlist.Clear();
            }
            
            Globals::Playlist.Enqueue(Shoutcast, Menu.GetSelection(), 
                    string(Shoutcast->StreamNames[Menu.GetSelection() - 1]));
            Globals::Playlist.Play();
            Globals::Display->RemoveTopScreen(&Menu);
            Globals::Remote.RemoveHandler();
            return;
            break;
    }
    return;
}

void ShoutcastSource::Play(unsigned int ID) {
    strcpy(StreamTitle, StreamNames[ID - 1]);
    
    /* Set title on status screen */
    Log::GetInstance()->Post(LOG_INFO, __FILE__, __LINE__,
            "Playing Streaming Station: %s", StreamTitle);
    Globals::Status->SetAttribs(GetTag(0));
    Globals::Display->Update(Globals::Status);
    
    ServerConn = OpenFile(StreamUrls[ID - 1][UrlPosition - 1]);
    
    if(Dec) {
        delete Dec;
    }
    Dec = new Mp3Decoder(ServerConn->GetDescriptor(), this);
    Dec->SetMetadataFrequency(MetadataFrequency);
    Dec->Start();
}

CommandHandler *ShoutcastSource::GetHandler(void) {
    return Handler;
}
