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
    int vfd_width = VFDLib::vfd_width;
    int vfd_height = VFDLib::vfd_height;
    int vfd_default_font = VFDLib::vfd_default_font;
    int StringHeight = Display.getTextHeight(vfd_default_font);
    int CurrentHeight = 0;
    int vfd_font_time = VFD_FONT_TIME;
    int timeX = 0;
    int timeY = 0;
    int randomX = 0;
    int randomY = 0;
    char randomText[] = "Random";

    pthread_mutex_lock(&ClassMutex);

    if (Globals::hw_type == HWTYP_EMPEG) /* Empeg can't fit time font here */
	    vfd_font_time = vfd_default_font;
    
    /* Set clipping area */
    Display.setClipArea(0, 0, vfd_width, vfd_height);
    
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
    Display.drawText(Title, 0, CurrentHeight, vfd_default_font, -1);
    
    CurrentHeight += BORDER_INFO + StringHeight;
    Display.drawLineHorizClipped(CurrentHeight, 0, vfd_width, VFDSHADE_DIM);

    CurrentHeight += BORDER_INFO + 1; /* '1' is the height of the horizontal line */
    Display.drawText(Artist, 0, CurrentHeight, vfd_default_font, -1);

    CurrentHeight += BORDER_INFO + StringHeight;
    Display.drawText(Album, 0, CurrentHeight, vfd_default_font, -1);
        
    /* Display elapsed time */
    char TimeString[9];
    sprintf(TimeString, "%d:%02d", Minutes, Seconds);

    timeX = vfd_width - BORDER_INFO - 
		Display.getTextWidth(TimeString, vfd_font_time);

    if (Globals::hw_type == HWTYP_EMPEG) /* Put time alongside album */ 
	timeY = CurrentHeight;
    else /* Put time at bottom right */
	timeY = vfd_height - 2 - Display.getTextHeight(vfd_font_time);
    
    /* Here we black out the area behind the track time */
    Display.drawSolidRectangleClipped( timeX, timeY,
            	vfd_width, vfd_height, VFDSHADE_BLACK);

    Display.drawText(TimeString, timeX, timeY, vfd_font_time, -1);
    
    if (Globals::hw_type == HWTYP_EMPEG) 
    { /* Put "Random" above time */
    	randomX = vfd_width - BORDER_INFO - 
	   Display.getTextWidth(randomText, VFD_FONT_SMALL);
    	randomY = timeY - BORDER_INFO - Display.getTextHeight(VFD_FONT_SMALL);
    }
    else
    { /* Put "Random" at bottom right of screen */
	randomX = 1;
	randomY = vfd_height - 2 - Display.getTextHeight(VFD_FONT_SMALL);
    }

    /* Show random status */
    if(Globals::Playlist.GetRandom() == true) {
        Display.drawText(randomText, randomX, randomY, VFD_FONT_SMALL, -1);
    }
    else {
        Display.drawSolidRectangleClipped(randomX, randomY,
                randomX + Display.getTextWidth(randomText, VFD_FONT_SMALL), 
		randomY + Display.getTextHeight(VFD_FONT_SMALL),
	       	VFDSHADE_BLACK);
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
