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

#ifndef PLAYLIST_HH
#define PLAYLIST_HH

#include "Tag.h"

class MenuScreen;

class Playlist {
public:
    Playlist(void);
    virtual ~Playlist(void);
    virtual void Advance(void);
    virtual void Reverse(void);
    virtual int GetPosition(void);
    virtual Tag GetTag(int EntryNumber) = 0;
    virtual char *GetFilename(char *Filename, int EntryNumber) = 0;
    virtual void SetMetadata(char *Metadata, int MetadataLength);
    virtual int CommandHandler(unsigned int Keycode, MenuScreen *ActiveMenu) = 0;

protected:
    int NumEntries;
    int Position;
        
private:
};
    
#endif /* #ifndef PLAYLIST_HH */
