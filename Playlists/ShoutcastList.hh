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

#ifndef SHOUTCASTLIST_HH
#define SHOUTCASTLIST_HH

#include "Playlist.hh"
#include "Tag.h"

class MenuScreen;

class ShoutcastList : public Playlist {
public:
    ShoutcastList(void);
    ~ShoutcastList(void);
    Tag GetTag(int EntryNumber);
    char *GetFilename(char *Filename, int EntryNumber);
    void SetMetadata(char *Metadata, int MetadataLength);
    int CommandHandler(unsigned int Keycode, MenuScreen *ActiveMenu);
    void Advance(void);
    void Reverse(void);
    int GetPosition(void);

private:
    char StreamTitle[256];
    char **StreamNames;
    char ***StreamUrls;
    int *NumUrls;
    int UrlPosition;
};

#endif /* #ifndef SHOUTCASTLIST_HH */
