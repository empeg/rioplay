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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <errno.h>
#include "RioServerSource.hh"
#include "Http.hh"
#include "Screen.hh"
#include "MenuScreen.hh"
#include "KeyCodes.h"
#include "Log.hh"
#include "Globals.hh"
#include "Mp3Decoder.hh"
#include "FlacDecoder.hh"
#include "VorbisDecoder.hh"
#include "CommandHandler.hh"
#include "MemAlloc.hh"

extern int errno;

StringID::StringID(void) {
}

StringID::~StringID(void) {
}

StringID::StringID(string inString, int inID) {
    Str = inString;
    ID = inID;
}

bool StringID::operator<(const StringID& SID) {
    return (Str < SID.Str);
}

RioServerSource::RioServerSource(void) {
    char SSDPRequest[] = "upnp:uuid:1D274DB0-F053-11d3-BF72-0050DA689B2F";
    struct sockaddr_in LocalAddr, BroadcastAddr;
    int SSDP;
    int One = 1;
    char TempString[256];
    FILE *fp;
    char *HttpLoc, *ColonLoc, *SlashLoc;
    Handler = new RioCommandHandler(this);
    
    /* Time to find the server... */
    if((SSDP = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        Log::GetInstance()->Post(LOG_FATAL, __FILE__, __LINE__,
                "socket() failed");
        return;
    }

    bzero((char *) &LocalAddr, sizeof(LocalAddr));
    LocalAddr.sin_family = AF_INET;
    LocalAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    LocalAddr.sin_port = htons(21075);
    bzero((char *) &BroadcastAddr, sizeof(BroadcastAddr));
    BroadcastAddr.sin_family = AF_INET;
    BroadcastAddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    BroadcastAddr.sin_port = htons(21075);

    if((bind(SSDP, (struct sockaddr *) &LocalAddr, sizeof(LocalAddr))) < 0)
    {
        Log::GetInstance()->Post(LOG_FATAL, __FILE__, __LINE__,
                "bind() failed: %s", strerror(errno));
        return;
    }

    if(setsockopt(SSDP, SOL_SOCKET, SO_BROADCAST, &One, sizeof(One)) < 0) {
        Log::GetInstance()->Post(LOG_FATAL, __FILE__, __LINE__,
                "setsockopt() failed: %s", strerror(errno));
        return;
    }
    
    if(sendto(SSDP, SSDPRequest, strlen(SSDPRequest), 0,
            (struct sockaddr *) &BroadcastAddr, sizeof(BroadcastAddr)) < 0) {
        Log::GetInstance()->Post(LOG_FATAL, __FILE__, __LINE__,
                "sendto() failed: %s", strerror(errno));
        return;
    }

    fp = fdopen(SSDP, "r");
    
    fgets(TempString, 256, fp);
    HttpLoc = strstr(TempString, "http://") + 7;
    ColonLoc = strstr(HttpLoc, ":") + 1;
    SlashLoc = strstr(ColonLoc, "/");
    memcpy(Server, HttpLoc, ColonLoc - HttpLoc - 1);
    Server[ColonLoc - HttpLoc - 1] = '\0';
    Port = atoi(ColonLoc);
    
    fclose(fp);
}

RioServerSource::~RioServerSource(void) {
    delete Handler;
}

void RioServerSource::DoQuery(char *Field, char *Query) {
    HttpConnection *Http = NULL;
    char TempString[256];
    char *TempPtr;
    FILE *fp;

    TempString[255] = '\0';    
    strcpy(SearchField, Field);
    
    /* Open a connection to the Rio HTTP Server */    
    sprintf(TempString, "http://%s:%d/query?%s=%s", Server, Port, Field, Query);
    Http = new HttpConnection(TempString);
    if(Http->Connect() < 0) {
        Log::GetInstance()->Post(LOG_FATAL, __FILE__, __LINE__,
                "HttpConnection::Connect() failed");
        return;
    }

    /* Find end of HTTP Header */
    Http->SkipHeader();
    
    /* Open descriptor as a file so we can use fgets */
    fp = Http->GetFilePointer();
    
    /* Throw away "matches=" response */
    fgets(TempString, 255, fp);

    /* Clear the list */
    List.clear();
    
    /* Read query responses */
    while(fgets(TempString, 255, fp) > 0) {
        /* fgets() does not remove trailing '\n', so 
           we need to take care of it */
        if(TempString[strlen(TempString) - 1] == '\n') {
            TempString[strlen(TempString) - 1] = '\0';
        }
        
        /* Get start pointer of actual title string */
        TempPtr = strstr(TempString, ":") + 1;
        
        /* Insert into list */
        List.push_back(string(TempPtr));
    }

    /* Close file descriptor and socket */
    delete Http;
}

void RioServerSource::DoResults(char *Field, const char *Query) {
    HttpConnection *Http = NULL;
    char TempString[256];
    char *TempPtr;
    int TempSongID;
    FILE *fp;
    
    /* Open a connection to the Rio HTTP Server */    
    sprintf(TempString, "http://%s:%d/results?%s=%s&_extended=1", Server, Port, Field, Query);
    Http = new HttpConnection(TempString);
    if(Http->Connect() < 0) {
        Log::GetInstance()->Post(LOG_FATAL, __FILE__, __LINE__,
                "HttpConnection::Connect() failed");
        return;
    }

    /* Find end of HTTP Header */
    Http->SkipHeader();
    
    /* Open descriptor as a file so we can use fgets */
    fp = Http->GetFilePointer();
    
    /* Clear the list */
    SongList.clear();
    
    /* Read the Song ID */
    while(fgets(TempString, 256, fp) > 0) {
        sscanf(TempString, "%x=", &TempSongID);
        TempPtr = strstr(TempString, "=") + 2;
        *(strstr(TempPtr, "\n")) = '\0';
        SongList.push_front(StringID(string(TempPtr), TempSongID));
    }

    /* Sort the list so it matches what was displayed on screen */
    SongList.sort();
    
    /* Close file descriptor and socket */
    delete Http;
}

void RioServerSource::DoPlaylists(void) {
    HttpConnection *Http = NULL;
    char TempString[256];
    char *TempPtr;
    FILE *fp;
    
    /* Open a connection to the Rio HTTP Server */    
    sprintf(TempString, "http://%s:%d/content/100?_extended=1", Server, Port);
    Http = new HttpConnection(TempString);
    if(Http->Connect() < 0) {
        Log::GetInstance()->Post(LOG_FATAL, __FILE__, __LINE__,
                "HttpConnection::Connect() failed");
        return;
    }

    /* Find end of HTTP Header */
    Http->SkipHeader();
    
    /* Open descriptor as a file so we can use fgets */
    fp = Http->GetFilePointer();

    /* Clear the list */
    SongList.clear();
    List.clear();
    
    /* Read query responses */
    while(fgets(TempString, 256, fp) > 0) {
        /* fgets() does not remove trailing '\n', so 
           we need to take care of it */
        if(TempString[strlen(TempString) - 1] == '\n') {
            TempString[strlen(TempString) - 1] = '\0';
        }
        
        /* Get start pointer of actual title string */
        TempPtr = strstr(TempString, "=") + 2;
        
        /* Assign */
        SongList.push_back(StringID(string(TempPtr), strtol(TempString, NULL, 16)));
        List.push_back(string(TempPtr));
    }

    /* Close file descriptor and socket */
    delete Http;
}

void RioServerSource::DoPlaylistContents(int ID) {
    HttpConnection *Http = NULL;
    char TempString[256];
    char *TempPtr;
    int TempSongID;
    FILE *fp;
    
    /* Open a connection to the Rio HTTP Server */    
    sprintf(TempString, "http://%s:%d/content/%x?_extended=1", Server, Port, ID);
    Http = new HttpConnection(TempString);
    if(Http->Connect() < 0) {
        Log::GetInstance()->Post(LOG_FATAL, __FILE__, __LINE__,
                "HttpConnection::Connect() failed");
        return;
    }

    /* Find end of HTTP Header */
    Http->SkipHeader();
    
    /* Open descriptor as a file so we can use fgets */
    fp = Http->GetFilePointer();
    
    /* Free SongID list */
    SongList.clear();
    
    /* Read the Song ID */
    while(fgets(TempString, 256, fp) > 0) {
        sscanf(TempString, "%x=", &TempSongID);
        TempPtr = strstr(TempString, "=") + 2;
        *(strstr(TempPtr, "\n")) = '\0';
        SongList.push_back(StringID(string(TempPtr), TempSongID));
    }

    /* Close file descriptor and socket */
    delete Http;
    
    /* Return the result */
    return;
}

Tag RioServerSource::GetTag(int ID) {
    HttpConnection *Http = NULL;
    int QueryFD;
    char TempString[256];
    Tag ReturnVal;
    char Data[256];
    unsigned char Key;
    unsigned char Size;

    /* Set to zeros to be safe */
    bzero(&ReturnVal, sizeof(ReturnVal));
    
    /* Open a connection to the Rio HTTP Server */    
    sprintf(TempString, "http://%s:%d/tags/%x", Server, Port, ID);
    Http = new HttpConnection(TempString);
    if((QueryFD = Http->Connect()) < 0) {
        Log::GetInstance()->Post(LOG_FATAL, __FILE__, __LINE__,
                "HttpConnection::Connect() failed");
        return ReturnVal;
    }

    /* Find end of HTTP Header */
    Http->SkipHeader();
    
    /* Parse tag data */
    while(read(QueryFD, &Key, 1)) {
        read(QueryFD, &Size, 1);
        read(QueryFD, Data, Size);
        
        switch(Key) {
            case TAG_KEY_TITLE:
                memcpy(ReturnVal.Title, Data, Size);
                break;
                
            case TAG_KEY_ARTIST:
                memcpy(ReturnVal.Artist, Data, Size);
                break;
                
            case TAG_KEY_ALBUM:
                memcpy(ReturnVal.Album, Data, Size);
                break;
                
            case TAG_KEY_YEAR:
                memcpy(ReturnVal.Year, Data, Size);
                break;
                
            case TAG_KEY_GENRE:
                memcpy(ReturnVal.Genre, Data, Size);
                break;
                
            case TAG_KEY_CODEC:
                memcpy(ReturnVal.Codec, Data, Size);
                break;
                
            default:
                break;
        }
    }

    /* Close file descriptor and socket */
    delete Http;    
    
    /* Return the array of results */
    return ReturnVal;
}

RioCommandHandler::RioCommandHandler(RioServerSource *inRio) {
    Rio = inRio;
}

RioCommandHandler::~RioCommandHandler(void) {
}

void RioCommandHandler::Handle(const unsigned long &Keycode) {
    list<StringID>::iterator IDiter;
    int Selection;
    
    if((Keycode == PANEL_MENU) || (Keycode == REMOTE_MENU)) {
        Globals::Display.RemoveTopScreen(&Menu);
        Globals::Remote.RemoveHandler();
        CurrentMenu = MENU_NONE;
        return;
    }
    else if((Keycode == PANEL_WHEEL_CW) || (Keycode == REMOTE_DOWN) || (Keycode == REMOTE_DOWN_REPEAT)) {
        Menu.Advance();
        Globals::Display.Update(&Menu);
        return;
    }
    else if((Keycode == PANEL_WHEEL_CCW) || (Keycode == REMOTE_UP) || (Keycode == REMOTE_UP_REPEAT)) {
        Menu.Reverse();
        Globals::Display.Update(&Menu);
        return;
    }

    switch(CurrentMenu) {
        case MENU_NONE: /* No Rio menu displayed yet */
            Menu.ClearOptions();
            Menu.SetTitle("Select Music");
            Menu.AddOption("Artist");
            Menu.AddOption("Album");
            Menu.AddOption("Genre");
            Menu.AddOption("Title");
            Menu.AddOption("Playlist");
            CurrentMenu = MENU_ROOT;
            Globals::Display.SetTopScreen(&Menu);
            Globals::Display.Update(&Menu);
            return;
            break;
            
        case MENU_ROOT:
            Selection = Menu.GetSelection();
            Menu.ClearOptions();
            CurrentMenu = MENU_SELECTFROMGROUP;
            Globals::Display.ShowHourglass();
            switch(Selection) {
                case 1:
                    Menu.SetTitle("Select Artist");
                    Menu.AddOption("Play All");
                    Rio->DoQuery("artist", "");
                    break;
                case 2:
                    Menu.SetTitle("Select Album");
                    Menu.AddOption("Play All");
                    Rio->DoQuery("source", "");
                    break;
                case 3:
                    Menu.SetTitle("Select Genre");
                    Menu.AddOption("Play All");
                    Rio->DoQuery("genre", "");
                    break;
                case 4:
                    Menu.SetTitle("Select Title");
                    Menu.AddOption("Play All");
                    Rio->DoQuery("title", "");
                    break;
                case 5:
                    Menu.SetTitle("Select Playlist");
                    Rio->DoPlaylists();
                    CurrentMenu = MENU_PLAYLIST;
                    break;
            }
            for(vector<string>::iterator iter = Rio->List.begin();
                    iter != Rio->List.end();
                    iter++) {
                Menu.AddOption((*iter).c_str());
            }
            Globals::Display.Update(&Menu);
            return;
            break;
            
        case MENU_SELECTFROMGROUP:
            Globals::Display.ShowHourglass();
            
            /* Clear the playlist if the user used the "Play" button
               (leave the playlist intact if "Enter" was used) */
            if((Keycode == REMOTE_PLAY) || (Keycode == PANEL_PLAY)) {
                Globals::Playlist.Clear();
            }
            
            Selection = Menu.GetSelection();
            if(Selection > 1) {
                HttpConnection::UrlEncode(Rio->List[Selection - 2]);
                Rio->DoResults(Rio->SearchField, Rio->List[Selection - 2].c_str());
            }
            else {
                Rio->DoResults(Rio->SearchField, "");
            }
            for(IDiter = Rio->SongList.begin();
                    IDiter != Rio->SongList.end(); IDiter++) {
                Globals::Playlist.Enqueue(Rio, (*IDiter).ID, (*IDiter).Str);
            }
            CurrentMenu = MENU_NONE;
            Globals::Display.RemoveTopScreen(&Menu);
            Globals::Display.ShowHourglass();
            Globals::Remote.RemoveHandler();
            Globals::Playlist.Play();
            return;
            break;
            
        case MENU_PLAYLIST:
            /* Clear the playlist if the user used the "Play" button
               (leave the playlist intact if "Enter" was used) */
            if((Keycode == REMOTE_PLAY) || (Keycode == PANEL_PLAY)) {
                Globals::Playlist.Clear();
            }
            
            IDiter = Rio->SongList.begin();
            /* Seems like there should be a better way to do this */
            for(int i = 0; i < (Menu.GetSelection() - 1); i++) {
                IDiter++;
            }
            Rio->DoPlaylistContents((*IDiter).ID);
            for(IDiter = Rio->SongList.begin(); IDiter != Rio->SongList.end();
                    IDiter++) {
                Globals::Playlist.Enqueue(Rio, (*IDiter).ID, (*IDiter).Str);
            }
            CurrentMenu = MENU_NONE;
            Globals::Display.RemoveTopScreen(&Menu);
            Globals::Display.ShowHourglass();
            Globals::Remote.RemoveHandler();
            Globals::Playlist.Play();
            return;
            break;
    }
    return;
}

void RioServerSource::Play(unsigned int ID) {
    char Filename[1024];
    sprintf(Filename, "http://%s:%d/content/%x", Server, Port, ID);
    
    ServerConn = OpenFile(Filename);
    
    /* If there's already a decoder running, kill it */
    if(Dec) {
        delete Dec;
        Dec = NULL;
    }
    
    /* Get track info */
    Tag TrackTag;
    TrackTag = GetTag(ID);
    
    /* Set title on status screen */
    Log::GetInstance()->Post(LOG_INFO, __FILE__, __LINE__,
            "Playing Title: %s Artist: %s Album: %s",
            TrackTag.Title, TrackTag.Artist, TrackTag.Album);
    Globals::Status.SetAttribs(TrackTag);
    Globals::Display.Update(&Globals::Status);
    
    /* Determine audio encoding type and create an instance of the 
       appropriate decoder */
    if(strcmp(TrackTag.Codec, "mp3") == 0) {
        Dec = new Mp3Decoder(ServerConn->GetDescriptor(), this);
    }
    else if(strcmp(TrackTag.Codec, "flac") == 0) {
        Dec = new FlacDecoder(ServerConn->GetDescriptor(), this);
    }
    else if(strcmp(TrackTag.Codec, "ogg") == 0) {
        Dec = new VorbisDecoder(ServerConn->GetDescriptor(), this);
    }
    else if(strcmp(TrackTag.Codec, "wma") == 0) {
        Log::GetInstance()->Post(LOG_ERROR, __FILE__, __LINE__,
                "WMA format is not supported (and probably won't ever be)");
        return;
    }
    else {
        Log::GetInstance()->Post(LOG_ERROR, __FILE__, __LINE__,
                "%s: Unknown codec", Filename);
        return;
    }
    
    /* Start the decoder process */
    Dec->Start();
}

CommandHandler *RioServerSource::GetHandler(void) {
    return Handler;
}
