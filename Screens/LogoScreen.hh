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

#ifndef LOGOSCREEN_HH
#define LOGOSCREEN_HH

#include "Screen.hh"
#include "SmallFont.hh"

class LogoScreen : public Screen {
public:
    LogoScreen(void);
    ~LogoScreen(void);
    virtual void Update(char *Display);
   
private:
    SmallFont LogoFont;
};

#endif /* #ifndef LOGOSCREEN_HH */
