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
#include <string.h>
#include "StatusScreen.hh"
#include "Tag.h"
#include "Globals.hh"
#include "MemAlloc.hh"

StatusScreen::StatusScreen(void) {
    Minutes = 0;
    Seconds = 0;
    Title[0] = '\0';
    Artist[0] = '\0';
    TitleArtistChanged = false;
    pthread_mutex_init(&ClassMutex, NULL);
}

StatusScreen::~StatusScreen(void) {
}


void StatusScreen::Update(VFDLib &Display) {
    pthread_mutex_lock(&ClassMutex);
    
    /* Set clipping area */
    Display.setClipArea(0, 0, 128, 64);
    
    if(TitleArtistChanged == true) {
        Display.clear(VFDSHADE_BLACK);
        TitleArtistChanged = false;
    }
    
    /* Display title, artist, and album */
    Display.drawText(Title, 0, 5, 1, -1);
    Display.drawLineHorizClipped(15, 0, 128, -1);
    Display.drawText(Artist, 0, 17, 1, -1);
    Display.drawText(Album, 0, 27, 1, -1);
        
    /* Display elapsed time */
    char TimeString[9];
    sprintf(TimeString, "%d:%02d", Minutes, Seconds);
    Display.drawSolidRectangleClipped(64, 62 - Display.getTextHeight(2),
            128, 64, VFDSHADE_BLACK);
    Display.drawText(TimeString, 126 - Display.getTextWidth(TimeString, 2),
            62 - Display.getTextHeight(2), 2, -1);
    
    /* Show random status */
    if(Globals::Playlist.GetRandom() == true) {
        Display.drawText("RANDOM", 1, 62 - Display.getTextHeight(0), 0, -1);
    }
    else {
        Display.drawSolidRectangleClipped(0, 62 - Display.getTextHeight(0) - 1,
                Display.getTextWidth("RANDOM", 0) + 2, 64, VFDSHADE_BLACK);
    }
    
    pthread_mutex_unlock(&ClassMutex);
}

void StatusScreen::SetTime(unsigned short NewMinutes, unsigned short NewSeconds) {
    pthread_mutex_lock(&ClassMutex);
    Minutes = NewMinutes;
    Seconds = NewSeconds;
    pthread_mutex_unlock(&ClassMutex);
}

void StatusScreen::SetAttribs(Tag TrackTag) {
    pthread_mutex_lock(&ClassMutex);
    strncpy(Title, TrackTag.Title, MAXSTRINGLENGTH);
    strncpy(Artist, TrackTag.Artist, MAXSTRINGLENGTH);
    strncpy(Album, TrackTag.Album, MAXSTRINGLENGTH);

    Title[MAXSTRINGLENGTH-1] = '\0';
    Artist[MAXSTRINGLENGTH-1] = '\0';
    Album[MAXSTRINGLENGTH-1] = '\0';
    
    TitleArtistChanged = true;
    pthread_mutex_unlock(&ClassMutex);
}
