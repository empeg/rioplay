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

#include <vector>
#include <string>
#include "Thread.hh"
#include "InputSource.hh"
#include "CommandHandler.hh"
#include "MenuScreen.hh"

#define COMMAND_NULL             0
#define COMMAND_PLAY             1
#define COMMAND_STOP             2
#define COMMAND_FORWARD          3
#define COMMAND_REVERSE          4
#define COMMAND_DECODER_FINISHED 5
#define COMMAND_CLEAR            6
#define COMMAND_PAUSE		 7

class PlaylistClass;

class PlaylistEntry {
public:
    InputSource *Source;
    unsigned int SourceID;
    string Title;
};

class PlaylistCommandHandler : public CommandHandler {
public:
    PlaylistCommandHandler(PlaylistClass *inPList);
    ~PlaylistCommandHandler(void);
    void Handle(const unsigned long &Keycode);
    int GetCurrentMenu() { return CurrentMenu; }
private:
    MenuScreen Menu;
    enum MenuTypes {
        MENU_NONE = 0,
        MENU_PLAYLIST
    };
        
    int CurrentMenu;
    PlaylistClass *PList;
};

class PlaylistClass : public Thread {
friend PlaylistCommandHandler;
public:
    PlaylistClass(void);
    ~PlaylistClass(void);
    void *ThreadMain(void *arg);
    
    void Enqueue(InputSource *Source, unsigned int SourceID, const string &Title);
    void Play(void);
    void Pause(void);
    void DecoderFinished(void);
    void Stop(bool Block = false);
    void Forward(void);
    void Reverse(void);
    void Clear(void);
    void Randomize(void);
    bool GetRandom(void);
    void SetRandom(void);
    CommandHandler *GetHandler(void);
    bool GetPlaying(void) { return isPlaying; }
/*    bool GetRepeat(void);
    void SetRepeat(bool inRepeat);*/
    
private:
//    void Dequeue(int InternalID);
    PlaylistEntry GetNextAndErase(void);
    void RequestCommand(int RequestedCommand, bool Block = false);
    vector<PlaylistEntry> Entries;
    vector<PlaylistEntry> PlayedEntries;
    PlaylistEntry CurrentlyPlayingEntry;
    pthread_cond_t NullCommandCondition;
    int Command;
    bool Random;
    PlaylistCommandHandler *Handler;
    bool isPlaying;
};

#endif /* #ifndef PLAYLIST_HH */
