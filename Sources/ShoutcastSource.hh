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

#ifndef SHOUTCASTSOURCE_HH
#define SHOUTCASTSOURCE_HH

#include "InputSource.hh"
#include "Tag.h"

class MenuScreen;

class ShoutcastSource : public InputSource {
public:
    ShoutcastSource(void);
    ~ShoutcastSource(void);
    Tag GetTag(int EntryNumber);
    Tag SetMetadata(char *Metadata, int MetadataLength);
    int CommandHandler(unsigned int Keycode, MenuScreen *ActiveMenu);
    void Play(unsigned int ID);

private:
    char StreamTitle[256];
    char **StreamNames;
    char ***StreamUrls;
    int *NumUrls;
    int UrlPosition;
};

#endif /* #ifndef SHOUTCASTSOURCE_HH */
