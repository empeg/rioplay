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

#ifndef SCREEN_HH
#define SCREEN_HH

class Screen {
public:
    Screen(void);
    virtual ~Screen(void);
    virtual void Update(char *Display) = 0;

protected:
    int DrawLetter(char *Display, char Letter, int x, int y);
    void DrawString(char *Display, char *Text, int x, int y);
    void CenterString(char *Display, char *Text, int y);
    void DrawHorizontalLine(char *Display, int x, int y, int length);
    void DrawVerticalLine(char *Display, int x, int y, int length);
    void Inverse(char *Display, int StartY, int EndY);
    void Inverse(char *Display, int TopLeftX, int TopLeftY, int BottomRightX, int BottomRightY);
};

#endif /* #ifndef SCREEN_HH */
