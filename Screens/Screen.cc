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
#include "Screen.hh"
#include "LetterBitmaps.h"

Screen::Screen(void) {
}

Screen::~Screen(void) {
}

int Screen::DrawLetter(char *Display, char Letter, int x, int y) {
    int tempx, tempy;
    unsigned char fillvalue;
    int Offset;
    
    if((x < 0) || (x > 127)) {
        return 0;
    }
    else if((y < 0) || (y > 63)) {
        return 0;
    }
    
    if((Letter >= 'A') && (Letter <= 'Z')) {
        Offset = Letter - 'A';
    }
    else if((Letter >= 'a') && (Letter <= 'z')) {
        Offset = Letter - 'a' + 26;
    }
    else if((Letter >= '0') && (Letter <= '9')) {
        Offset = Letter - '0' + 52;
    }
    else if(Letter == ' ') {
        return 4;
    }
    else {
        /* Unsupported character */
        return 0;
    }
    
    for(tempy = y; tempy < y + 14; tempy++) {
        for(tempx = x; (tempx < x + LetterWidth[Offset]) && (tempx < 128); tempx++) {
            if(tempx & 1)
                fillvalue = 0xf0;
            else
                fillvalue = 0x0f;

            if(LetterBitmaps[Offset][(tempx - x) + ((tempy - y) * 13)] == 0) {
                /* Pixel shouuld be filled */
                Display[(tempx >> 1) + (64 * tempy)] |= fillvalue;
            }
            else {
                /* Pixel should be clear */
                Display[(tempx >> 1) + (64 * tempy)] &= ~(fillvalue);
            }
        }
    }
    
    return LetterWidth[Offset];
}

void Screen::DrawString(char *Display, char *Text, int x, int y) {
    unsigned int i;
    
    for(i = 0; i < strlen(Text); i++) {
        x += (DrawLetter(Display, Text[i], x, y) + 1);
    }
}

void Screen::DrawHorizontalLine(char *Display, int x, int y, int length) {
    int i;
    unsigned char fillvalue;
    
    for(i = 0; i < length; i++) {
        if(i & 1)
            fillvalue = 0xf0;
        else
            fillvalue = 0x0f;
        
        Display[((x + i) >> 1) + (64 * y)] |= fillvalue;
    }
}

void Screen::DrawVerticalLine(char *Display, int x, int y, int length) {
    int i;
    unsigned char fillvalue;
    
    for(i = 0; i < length; i++) {
        if(x & 1)
            fillvalue = 0xf0;
        else
            fillvalue = 0x0f;
        
        Display[(x >> 1) + (64 * (y + i))] |= fillvalue;
    }
}    

void Screen::CenterString(char *Display, char *Text, int y) {
    int StringWidth = 0;
    int Offset, x;
    unsigned int i;
    
    for(i = 0; i < strlen(Text); i++) {
        if((Text[i] >= 'A') && (Text[i] <= 'Z')) {
            Offset = Text[i] - 'A';
        }
        else if((Text[i] >= 'a') && (Text[i] <= 'z')) {
            Offset = Text[i] - 'a' + 26;
        }
        else if((Text[i] >= '0') && (Text[i] <= '9')) {
            Offset = Text[i] - '0' + 52;
        }
        else if(Text[i] == ' ') {
            Offset = 4;
        }
        else {
            Offset = 0;
        }
        StringWidth += (LetterWidth[Offset] + 1);
    }
    
    /* Correct for the 1 extra pixel we added at the end */
    StringWidth--;
    x = (128 - StringWidth) / 2;
    
    DrawString(Display, Text, x, y);
}

void Screen::Inverse(char *Display, int TopLeftX, int TopLeftY, int BottomRightX, int BottomRightY) {
    int x, y;
    unsigned char fillmask;
    
    for(y = TopLeftY; y <= BottomRightY; y++) {
        for(x = TopLeftX; x <= BottomRightX; x++) {
            if(x & 1)
                fillmask = 0xf0;
            else
                fillmask = 0x0f;

            if(fillmask & Display[(x >> 1) + (64 * y)]) {
                Display[(x >> 1) + (64 * y)] &= ~fillmask;
            }
            else {
                Display[(x >> 1) + (64 * y)] |= fillmask;
            }
        }
    }
}
