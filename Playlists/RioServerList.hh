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

#ifndef RIOSERVERLIST_H
#define RIOSERVERLIST_H

#include "Playlist.hh"
#include "Tag.h"

class MenuScreen;

class RioServerList : public Playlist {
public:
    RioServerList(void);
    ~RioServerList(void);
    virtual void Advance(void);
    virtual void Reverse(void);
    virtual int GetPosition(void);
    virtual Tag GetTag(int EntryNumber);
    virtual char *GetFilename(char *Filename, int EntryNumber);
    void DoQuery(char *Field, char *Query);
    int CommandHandler(unsigned int Keycode, MenuScreen *ActiveMenu);

private:    
    void DoResults(char *Field, char *Query);
    void DoPlaylists(void);
    void DoPlaylistContents(int ID);
    
    char **List;              /* Array of results returned from server */
    char Server[64];          /* Name of Rio Server */
    unsigned short Port;      /* Port of Rio Server */
    int *SongID;              /* Current SongID */
    int NumSongIDEntries;
    int SongIDPosition;
    char SearchField[8];
};

#endif /* #ifndef RIOSERVERLIST_H */
