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

#ifndef STATUSSCREEN_HH
#define STATUSSCREEN_HH

#include <pthread.h>
#include "Screen.hh"
#include "Tag.h"
#include "VFDLib.hh"

#define MAXSTRINGLENGTH 128

class StatusScreen : public Screen {
public:
    StatusScreen(void);
    ~StatusScreen(void);
    virtual void Update(VFDLib &Display);
    void SetTime(unsigned short NewMinutes, unsigned short NewSeconds);
    void SetAttribs(Tag TrackTag);
    
private:
    unsigned short Minutes;
    unsigned short Seconds;
    char Title[MAXSTRINGLENGTH];
    char Artist[MAXSTRINGLENGTH];
    char Album[MAXSTRINGLENGTH];
    bool TitleArtistChanged;
    pthread_mutex_t ClassMutex;
};

#endif /* #ifndef STATUSSCREEN_HH */
