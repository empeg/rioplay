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
#include "CommandHandler.hh"
#include "MenuScreen.hh"

class CommandHandler;
class RioServerSource;

class StringID {
public:
    StringID(void);
    ~StringID(void);
    StringID(string inString, int inID);
    bool operator<(const StringID& SID);
    string Str;
    int ID;
};

class RioCommandHandler : public CommandHandler {
public:
    RioCommandHandler(RioServerSource *inRio);
    ~RioCommandHandler(void);
    void Handle(const unsigned long &Keycode);

private:
    MenuScreen Menu;
    enum MenuTypes {
        MENU_NONE = 0,
        MENU_ROOT,
        MENU_SELECTFROMGROUP,
        MENU_PLAYLIST
    };
        
    int CurrentMenu;
    RioServerSource *Rio;
};

class RioServerSource : public InputSource {
friend RioCommandHandler;
public:
    RioServerSource(void);
    ~RioServerSource(void);
    void Play(unsigned int ID);
    Tag GetTag(int ID);
    void DoQuery(char *Field, char *Query);
    CommandHandler *GetHandler(void);

private:    
    void DoResults(char *Field, const char *Query);
    void DoPlaylists(void);
    void DoPlaylistContents(int ID);
    
    char Server[64];          /* Name of Rio Server */
    unsigned short Port;      /* Port of Rio Server */
    char SearchField[8];
    list<StringID> SongList;
    vector<string> List;
    RioCommandHandler *Handler;
};

#endif /* #ifndef RIOSERVERSOURCE_H */
