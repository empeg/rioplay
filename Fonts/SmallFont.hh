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

#ifndef SMALLFONT_HH
#define SMALLFONT_HH

#include "Font.hh"

class SmallFont : public Font {
public:
    SmallFont(void);
    ~SmallFont(void);

protected:        
    virtual int GetCharWidth(char Character);
    virtual int DrawChar(char *Display, char Character, int x, int y);

private:
    int ComputeOffset(char Character);
    int XOffset[128];
};

#endif /* #ifndef SMALLFONT_HH */
