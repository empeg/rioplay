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
#include "VFDLib.hh"

class LogoScreen : public Screen {
public:
    LogoScreen(void);
    ~LogoScreen(void);
    virtual void Update(VFDLib &Display);
   
private:
};

#endif /* #ifndef LOGOSCREEN_HH */
