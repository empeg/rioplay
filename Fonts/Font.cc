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

#include <string.h>
#include "Font.hh"
#include "MemAlloc.hh"

Font::Font(void) {
}

Font::~Font(void) {
}


void Font::DrawString(char *Display, char *Text, int x, int y) {
    unsigned int i;
    
    for(i = 0; i < strlen(Text); i++) {
        x += (DrawChar(Display, Text[i], x, y) + 1);
    }
}

void Font::DrawStringCentered(char *Display, char *Text, int y) {
    int StringWidth = 0;
    int x;
    unsigned int i;
    
    for(i = 0; i < strlen(Text); i++) {
        StringWidth += (GetCharWidth(Text[i]) + 1);
    }
    
    /* Correct for the 1 extra pixel we added at the end */
    StringWidth--;
    x = (128 - StringWidth) / 2;
    
    DrawString(Display, Text, x, y);
}
