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

#ifndef FONT_HH
#define FONT_HH

#include <stdio.h>

class Font {
public:
    Font(void);
    virtual ~Font(void);
    int GetHeight(void);
    void DrawString(char *Display, char *Text, int x, int y);
    void DrawStringCentered(char *Display, char *Text, int y);
 
protected:
    virtual int GetCharWidth(char Character) = 0;
    virtual int DrawChar(char *Display, char Character, int x, int y) = 0;
    void DrawPixel(char *Display, bool Set, int x, int y);
    int Height;
    int CharWidth[128];
};

inline int Font::GetHeight(void) {
    return Height;
}

inline void Font::DrawPixel(char *Display, bool Set, int x, int y) {
    unsigned char fillvalue;
    
    if(x & 1)
        fillvalue = 0xf0;
    else
        fillvalue = 0x0f;

    if(Set) {
        /* Pixel should be filled */
        Display[(x >> 1) + (64 * y)] |= fillvalue;
    }
    else {
        /* Pixel should be clear */
        Display[(x >> 1) + (64 * y)] &= ~(fillvalue);
    }
}

#endif /* #ifndef FONT_HH */
