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

#ifndef RIOSERVERSOURCE_H
#define RIOSERVERSOURCE_H

#include <vector>
#include <list>
#include <string>
#include "InputSource.hh"
#include "Tag.h"

class MenuScreen;

class StringID {
public:
    StringID(void);
    ~StringID(void);
    StringID(string inString, int inID);
    bool operator<(const StringID& SID);
    string Str;
    int ID;
};

class RioServerSource : public InputSource {
public:
    RioServerSource(void);
    ~RioServerSource(void);
    virtual void Play(unsigned int ID);
    virtual Tag GetTag(int ID);
    void DoQuery(char *Field, char *Query);
    int CommandHandler(unsigned int Keycode, MenuScreen *ActiveMenu);

private:    
    void DoResults(char *Field, const char *Query);
    void DoPlaylists(void);
    void DoPlaylistContents(int ID);
    
    char Server[64];          /* Name of Rio Server */
    unsigned short Port;      /* Port of Rio Server */
    char SearchField[8];
    list<StringID> SongList;
    vector<string> List;
};

#endif /* #ifndef RIOSERVERSOURCE_H */
