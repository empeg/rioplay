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

#include <unistd.h>
#include <sys/time.h>
#include "Playlist.hh"
#include "Globals.hh"
#include "MemAlloc.hh"

PlaylistClass::PlaylistClass(void) {
    CurrentlyPlayingEntry.Source = NULL;
    CurrentlyPlayingEntry.SourceID = 0;
    pthread_cond_init(&NullCommandCondition, NULL);
    Command = COMMAND_NULL;
    
    /* Initialize random number generator */
    struct timeval tv;
    gettimeofday(&tv, NULL);
    srand((int) tv.tv_usec);
}

PlaylistClass::~PlaylistClass(void) {
}

void *PlaylistClass::ThreadMain(void *arg) {
    pthread_mutex_lock(&ClassMutex);
    while(1) {
        /* Wait for an event */
        pthread_cond_wait(&ClassCondition, &ClassMutex);
        
        switch(Command) {
            case COMMAND_PLAY:
                if(CurrentlyPlayingEntry.Source == NULL) {
                    if(Entries.size() > 0) {
                        /* Store entry we're about to play in a
                           temporary location */
                        CurrentlyPlayingEntry = Entries[0];

                        /* Remove it from the "to be played" list */
                        Entries.erase(Entries.begin());

                        /* Go ahead and play it */
                        CurrentlyPlayingEntry.Source->Play(CurrentlyPlayingEntry.SourceID);
                    }
                    else {
                        /* No entries in playlist; do nothing */
                    }
                }
                else {
                    /* We're already playing something so just ignore this
                       command */
                }
                break;
                
            case COMMAND_STOP:
                if(CurrentlyPlayingEntry.Source != NULL) {
                    /* Stop playing the current song */
                    CurrentlyPlayingEntry.Source->Stop();
                    
                    /* Place this song back on the "to be played" list
                       since it wasn't finished playing */
                    Entries.insert(Entries.begin(), CurrentlyPlayingEntry);
                    
                    /* Clear out the currently playing struct since nothing
                       is playing now */
                    CurrentlyPlayingEntry.Source = NULL;
                    CurrentlyPlayingEntry.SourceID = 0;
                }
                break;
                
            case COMMAND_FORWARD:
                if(CurrentlyPlayingEntry.Source != NULL) {
                    /* Stop playing the current song */
                    CurrentlyPlayingEntry.Source->Stop();
                    
                    /* Show hourglass */
                    Globals::Display.ShowHourglass();
                    
                    /* Push the song that was just playing on
                       the "played" list */
                    PlayedEntries.push_back(CurrentlyPlayingEntry);
                    
                    if(Entries.size() > 0) {
                        /* Store entry we're about to play in a temporary
                           location */
                        CurrentlyPlayingEntry = Entries[0];

                        /* Remove it from the "to be played" list */
                        Entries.erase(Entries.begin());

                        /* Go ahead and play it */
                        CurrentlyPlayingEntry.Source->Play(CurrentlyPlayingEntry.SourceID);
                    }
                    else {
                        /* Clear out the currently playing struct since nothing
                           is playing now */
                        CurrentlyPlayingEntry.Source = NULL;
                        CurrentlyPlayingEntry.SourceID = 0;
                    }
                }
                break;
                
            case COMMAND_REVERSE:
                if(CurrentlyPlayingEntry.Source != NULL) {
                    /* Stop playing the current song */
                    CurrentlyPlayingEntry.Source->Stop();
                    
                    /* Show hourglass */
                    Globals::Display.ShowHourglass();
                    
                    /* Push the song that was just playing on
                       the "to be played" list */
                    Entries.insert(Entries.begin(), CurrentlyPlayingEntry);
                    
                    if(PlayedEntries.size() > 0) {
                        /* Store entry we're about to play in a temporary
                           location */
                        CurrentlyPlayingEntry = PlayedEntries[PlayedEntries.size() - 1];

                        /* Remove it from the "already played" list */
                        PlayedEntries.erase(PlayedEntries.end());

                        /* Go ahead and play it */
                        CurrentlyPlayingEntry.Source->Play(CurrentlyPlayingEntry.SourceID);
                    }
                    else {
                        /* Clear out the currently playing struct since nothing
                           is playing now */
                        CurrentlyPlayingEntry.Source = NULL;
                        CurrentlyPlayingEntry.SourceID = 0;
                    }
                }
                break;
                
            case COMMAND_DECODER_FINISHED:
                /* Finished playing whatever was currently playing */
                
                /* Push the song that was just playing on the "played" list */
                PlayedEntries.push_back(CurrentlyPlayingEntry);
                
                /* Play the next song in the list */
                if(Entries.size() > 0) {
                    /* Store entry we're about to play in a temporary location */
                    CurrentlyPlayingEntry = Entries[0];
                    
                    /* Remove it from the "to be played" list */
                    Entries.erase(Entries.begin());
                    
                    /* Go ahead and play it */
                    CurrentlyPlayingEntry.Source->Play(CurrentlyPlayingEntry.SourceID);
                }
                else {
                    /* No entries in "to be played" playlist;
                       should check "repeat" and move "played" songs
                       to "to be played" list and start over */
                    CurrentlyPlayingEntry.Source = NULL;
                    CurrentlyPlayingEntry.SourceID = 0;
                    
                    /* Put all the played songs back into the "to be played"
                       list */
                    Entries.swap(PlayedEntries);
                }
                break;
                
            case COMMAND_CLEAR:
                if(CurrentlyPlayingEntry.Source != NULL) {
                    /* Stop playing the current song */
                    CurrentlyPlayingEntry.Source->Stop();
                    CurrentlyPlayingEntry.Source = NULL;
                    CurrentlyPlayingEntry.SourceID = 0;
                }
                Entries.clear();
                PlayedEntries.clear();
                break;
                    
            default:
                break;
        }
        
        /* Indicate that command was handled */
        Command = COMMAND_NULL;
        
        /* Signal threads which are waiting to request a command */
        pthread_cond_signal(&NullCommandCondition);
    }
    pthread_mutex_unlock(&ClassMutex);
}

void PlaylistClass::Enqueue(InputSource *Source, unsigned int SourceID) {
    PlaylistEntry TempEntry;
    
    TempEntry.Source = Source;
    TempEntry.SourceID = SourceID;
    
    pthread_mutex_lock(&ClassMutex);
    Entries.push_back(TempEntry);
    pthread_mutex_unlock(&ClassMutex);
}

void PlaylistClass::RequestCommand(int RequestedCommand, bool Block = false) {
    pthread_mutex_lock(&ClassMutex);
    
    /* Wait until any previously requested commands are processed */
    while(Command != COMMAND_NULL) {
        /* Signal playlist thread that command is waiting in case it
           missed the previous signal */
        pthread_cond_signal(&ClassCondition);
        pthread_cond_wait(&NullCommandCondition, &ClassMutex);
    }
    
    /* Request that playback be started */
    Command = RequestedCommand;
    pthread_cond_signal(&ClassCondition);

    if(Block) {
        /* Wait for command to be processed */
        while(Command != COMMAND_NULL) {
            pthread_cond_signal(&ClassCondition);
            pthread_cond_wait(&NullCommandCondition, &ClassMutex);
        }
    }
        
    pthread_mutex_unlock(&ClassMutex);
}

void PlaylistClass::Play(void) {
    RequestCommand(COMMAND_PLAY);
}

void PlaylistClass::DecoderFinished(void) {
    RequestCommand(COMMAND_DECODER_FINISHED);
}

void PlaylistClass::Stop(bool Block = false) {
    RequestCommand(COMMAND_STOP, Block);
}

void PlaylistClass::Forward(void) {
    RequestCommand(COMMAND_FORWARD);
}

void PlaylistClass::Reverse(void) {
    RequestCommand(COMMAND_REVERSE);
}

void PlaylistClass::Clear(void) {
    RequestCommand(COMMAND_CLEAR);
}

void PlaylistClass::Randomize(void) {
    vector<PlaylistEntry> TempEntries;
    int i;
    vector<PlaylistEntry>::iterator iter;
    
    pthread_mutex_lock(&ClassMutex);
    
    while(Entries.size() > 0) {
        /* Get a random entry out of the "to be played" list */
        i = 1 + (int) ((Entries.size() * 1.0) * rand() / (RAND_MAX + 1.0));
        
        /* Add it to the end of the temp list */
        TempEntries.push_back(Entries[i]);
        
        /* Erase from the "to be played" list */
        iter = Entries.begin();
        iter += i;
        Entries.erase(iter);
    }

    /* Copy randomized temp list back into the "to be played" list */
    Entries = TempEntries;
        
    pthread_mutex_unlock(&ClassMutex);
}
