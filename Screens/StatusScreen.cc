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
    int StringHeight = Display.getTextHeight(VFD_DEFAULT_FONT);
    int CurrentHeight = 0;
    pthread_mutex_lock(&ClassMutex);
    
    /* Set clipping area */
    Display.setClipArea(0, 0, VFD_WIDTH, VFD_HEIGHT);
    
    if(TitleArtistChanged == true) {
        Display.clear(VFDSHADE_BLACK);
        TitleArtistChanged = false;
    }
    
    /* Display title, artist, and album */
    /* We supply a common border to seperate the lines of text, and compute the y-axis
     * based on this, to allow for differences in font height across various platforms.
     */
    #define BORDER_INFO 3
    CurrentHeight = BORDER_INFO;
    Display.drawText(Title, 0, CurrentHeight, VFD_DEFAULT_FONT, -1);
    
    CurrentHeight += BORDER_INFO + StringHeight;
    Display.drawLineHorizClipped(CurrentHeight, 0, VFD_WIDTH, -1);

    CurrentHeight += BORDER_INFO + 1; /* '1' is the height of the horizontal line */
    Display.drawText(Artist, 0, CurrentHeight, VFD_DEFAULT_FONT, -1);

    CurrentHeight += BORDER_INFO + StringHeight;
    Display.drawText(Album, 0, CurrentHeight, VFD_DEFAULT_FONT, -1);
        
    /* Display elapsed time */
    char TimeString[9];
    sprintf(TimeString, "%d:%02d", Minutes, Seconds);

    /* Here we black out the area behind the track time */
    Display.drawSolidRectangleClipped(VFD_WIDTH-2 - Display.getTextWidth(TimeString, VFD_FONT_TIME), 
		VFD_HEIGHT-2 - Display.getTextHeight(VFD_FONT_TIME),
            	VFD_WIDTH, VFD_HEIGHT, VFDSHADE_BLACK);

    Display.drawText(TimeString, VFD_WIDTH-2 - Display.getTextWidth(TimeString, VFD_FONT_TIME),
            VFD_HEIGHT-2 - Display.getTextHeight(VFD_FONT_TIME), VFD_FONT_TIME, -1);
    
    /* Show random status */
    if(Globals::Playlist.GetRandom() == true) {
        Display.drawText("RANDOM", 1, VFD_HEIGHT-2 - Display.getTextHeight(VFD_FONT_SMALL), 
		VFD_FONT_SMALL, -1);
    }
    else {
        Display.drawSolidRectangleClipped(0, VFD_HEIGHT-2 - Display.getTextHeight(VFD_FONT_SMALL) - 1,
                Display.getTextWidth("RANDOM", VFD_FONT_SMALL) + 2, VFD_HEIGHT, VFDSHADE_BLACK);
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
