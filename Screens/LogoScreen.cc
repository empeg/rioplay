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


void LogoScreen::Update(char *Display) {
    int x, y;
    unsigned char fillvalue;
    char VersionString[24];
    
    for(y = 0; y < 64; y++) {
        for(x = 0; x < 128; x++) {
            if(x & 1)
                fillvalue = 0xf0;
            else
                fillvalue = 0x0f;

            if(RioPlayLogo[x + (y * 128)] == 0) {
                /* Pixel shouuld be filled */
                Display[(x >> 1) + (64 * y)] |= fillvalue;
            }
            else {
                /* Pixel should be clear */
                Display[(x >> 1) + (64 * y)] &= ~(fillvalue);
            }
        }
    }
    sprintf(VersionString, "Version %s", PLAYER_VER);
    LogoFont.DrawStringCentered(Display, VersionString, 54);
}
