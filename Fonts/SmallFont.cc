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
#include "SmallFont.hh"
#include "SmallFontBitmap.h"
#include "MemAlloc.hh"

SmallFont::SmallFont(void) {
    int x, y;
    bool gap;
    int LetterIndex = 0;
    int LastGap = -1;
    
    Height = 9;
    for(x = 0; x < FONT_BITMAP_WIDTH; x++) {
        gap = true;
        for(y = 0; y < Height; y++) {
            if(SmallFontBitmap[(y * FONT_BITMAP_WIDTH) + x] == '*') {
                gap = false;
            }
        }
        if(gap) {
            CharWidth[LetterIndex] = x - LastGap - 1;
            XOffset[LetterIndex] = LastGap + 1;
            LetterIndex++;
            LastGap = x;
        }
    }
}

SmallFont::~SmallFont(void) {
}

int SmallFont::GetCharWidth(char Character) {
    int Offset;
    
    Offset = ComputeOffset(Character);
    if(Offset != -1) {
        return CharWidth[Offset];
    }
    else {
        /* Unsupported character */
        return 0;
    }
}

int SmallFont::DrawChar(char *Display, char Character, int x, int y) {
    int tempx, tempy;
    int Offset;
    
    Offset = ComputeOffset(Character);
    if(Offset == -1) {
        return 3;
    }
        
    for(tempy = 0; (tempy < Height) && (y + tempy < 64); tempy++) {
        for(tempx = 0; (tempx < CharWidth[Offset])
                && (x + tempx < 128); tempx++) {
            if(SmallFontBitmap[(tempy * FONT_BITMAP_WIDTH) + (tempx + XOffset[Offset])] == '*') {
                DrawPixel(Display, true, x + tempx, y + tempy);
            }
            else {
                DrawPixel(Display, false, x + tempx, y + tempy);
            }
        }
    }
    
    return CharWidth[ComputeOffset(Character)];
}

inline int SmallFont::ComputeOffset(char Character) {
    int Offset;
    
    if((Character >= 'A') && (Character <= 'Z')) {
        Offset = Character - 'A';
    }
    else if((Character >= 'a') && (Character <= 'z')) {
        Offset = Character - 'a' + 26;
    }
    else if((Character >= '0') && (Character <= '9')) {
        Offset = Character - '0' + 52;
    }
    else if((Character >= '!') && (Character <= '/')) {
        Offset = Character - '!' + 62;
    }
    else if((Character >= ':') && (Character <= '@')) {
        Offset = Character - ':' + 77;
    }
    else {
        /* Unsupported character */
        return -1;
    }
    
    return Offset;
}
