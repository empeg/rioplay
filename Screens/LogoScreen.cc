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

    Display.setClipArea(0, 0, 128, 64);
    Display.clear(VFDSHADE_BLACK);
    Display.drawBitmap(RioPlayLogo, 0, 0, 0, 0, 128, 64, VFDSHADE_BRIGHT, 0);
    sprintf(VersionString, "Version %s", PLAYER_VER);
    Display.drawText(VersionString, (128 - Display.getTextWidth(VersionString, 1)) / 2, 54, 1, -1);
}
