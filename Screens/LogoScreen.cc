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
#include "LogoScreen.hh"
#include "RioPlay-logo.h"
#include "Player.h"
#include "MemAlloc.hh"

LogoScreen::LogoScreen(void) {
}

LogoScreen::~LogoScreen(void) {
}


void LogoScreen::Update(VFDLib &Display) {
    char VersionString[24];
    int vfd_width = VFDLib::vfd_width;
    int vfd_height = VFDLib::vfd_height;
    int vfd_default_font = VFDLib::vfd_default_font;
    unsigned char * riologo = RioPlayLogo;
    
    if (vfd_height < 64) /* Smaller screen, use smaller logo */
    	riologo = RioPlayLogoEmpeg;
	
    /* We compute the y-axis to accomodate different VFD heights on different platforms */
    int y = vfd_height - Display.getTextHeight(vfd_default_font); 

    Display.setClipArea(0, 0, vfd_width, vfd_height);
    Display.clear(VFDSHADE_BLACK);
    Display.drawBitmap(riologo, 0, 0, 0, 0, 
		vfd_width, vfd_height, VFDSHADE_BRIGHT, 0);
    sprintf(VersionString, "Version %s", PLAYER_VER);
    Display.drawText(VersionString, (vfd_width - 
		Display.getTextWidth(VersionString, vfd_default_font)) / 2, 
		y, vfd_default_font, -1);
}
