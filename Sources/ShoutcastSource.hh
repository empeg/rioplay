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
#include "CommandHandler.hh"
#include "MenuScreen.hh"

class CommandHandler;
class ShoutcastSource;

class ShoutcastCommandHandler : public CommandHandler {
public:
    ShoutcastCommandHandler(ShoutcastSource *inShoutcast);
    ~ShoutcastCommandHandler(void);
    void Handle(const unsigned long &Keycode);

private:
    MenuScreen Menu;
    enum MenuTypes {
        MENU_NONE = 0,
        MENU_STREAM,
        MENU_SELECTFROMGROUP,
        MENU_PLAYLIST
    };
        
    int CurrentMenu;
    ShoutcastSource *Shoutcast;
};

class ShoutcastSource : public InputSource {
friend ShoutcastCommandHandler;
public:
    ShoutcastSource(void);
    ~ShoutcastSource(void);
    Tag GetTag(int EntryNumber);
    Tag SetMetadata(char *Metadata, int MetadataLength);
    void Play(unsigned int ID);
    CommandHandler *GetHandler(void);

private:
    char StreamTitle[256];
    char **StreamNames;
    char ***StreamUrls;
    int *NumUrls;
    int UrlPosition;
    ShoutcastCommandHandler *Handler;
};

#endif /* #ifndef SHOUTCASTSOURCE_HH */
