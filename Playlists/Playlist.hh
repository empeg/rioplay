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
#include "Thread.hh"
#include "InputSource.hh"

#define COMMAND_NULL             0
#define COMMAND_PLAY             1
#define COMMAND_STOP             2
#define COMMAND_FORWARD          3
#define COMMAND_REVERSE          4
#define COMMAND_DECODER_FINISHED 5
#define COMMAND_CLEAR            6

class PlaylistEntry {
public:
    InputSource *Source;
    unsigned int SourceID;
};

class PlaylistClass : public Thread {
public:
    PlaylistClass(void);
    ~PlaylistClass(void);
    void *ThreadMain(void *arg);
    
    void Enqueue(InputSource *Source, unsigned int SourceID);
    void Play(void);
    void DecoderFinished(void);
    void Stop(bool Block = false);
    void Forward(void);
    void Reverse(void);
    void Clear(void);
    void Randomize(void);
/*    bool GetRandom(void);
    void SetRandom(bool inRandom);
    bool GetRepeat(void);
    void SetRepeat(bool inRepeat);*/
    
private:
//    void Dequeue(int InternalID);
    void RequestCommand(int RequestedCommand, bool Block = false);
    vector<PlaylistEntry> Entries;
    vector<PlaylistEntry> PlayedEntries;
    PlaylistEntry CurrentlyPlayingEntry;
    pthread_cond_t NullCommandCondition;
    int Command;
};

#endif /* #ifndef PLAYLIST_HH */
