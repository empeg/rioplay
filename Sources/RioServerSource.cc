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
#include "MemAlloc.hh"

extern int errno;
extern AudioOutputDevice PlaylistAudioOut;

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

int RioServerSource::CommandHandler(unsigned int Keycode, MenuScreen *ActiveMenu) {
    static int CurrentMenu = 0;
    list<StringID>::iterator IDiter;
    int Selection;
    
    if((Keycode == PANEL_MENU) || (Keycode == REMOTE_MENU)) {
        Globals::Display.RemoveTopScreen(ActiveMenu);
        CurrentMenu = 0;
        return 0;
    }
    else if((Keycode == PANEL_WHEEL_CW) || (Keycode == REMOTE_DOWN) || (Keycode == REMOTE_DOWN_REPEAT)) {
        ActiveMenu->Advance();
        Globals::Display.Update(ActiveMenu);
        return 1;
    }
    else if((Keycode == PANEL_WHEEL_CCW) || (Keycode == REMOTE_UP) || (Keycode == REMOTE_UP_REPEAT)) {
        ActiveMenu->Reverse();
        Globals::Display.Update(ActiveMenu);
        return 1;
    }

    switch(CurrentMenu) {
        case 0: /* Rio Server Playlist root menu */
            ActiveMenu->ClearOptions();
            ActiveMenu->SetTitle("Select Music");
            ActiveMenu->AddOption("Artist");
            ActiveMenu->AddOption("Album");
            ActiveMenu->AddOption("Genre");
            ActiveMenu->AddOption("Title");
            ActiveMenu->AddOption("Playlist");
            CurrentMenu = 1;
            Globals::Display.Update(ActiveMenu);
            return 1;
            break;
            
        case 1:
            Selection = ActiveMenu->GetSelection();
            ActiveMenu->ClearOptions();
            ActiveMenu->SetTitle("Select Artist");
            CurrentMenu = 2;
            Globals::Display.ShowHourglass();
            switch(Selection) {
                case 1:
                    ActiveMenu->AddOption("Play All");
                    DoQuery("artist", "");
                    break;
                case 2:
                    ActiveMenu->AddOption("Play All");
                    DoQuery("source", "");
                    break;
                case 3:
                    ActiveMenu->AddOption("Play All");
                    DoQuery("genre", "");
                    break;
                case 4:
                    ActiveMenu->AddOption("Play All");
                    DoQuery("title", "");
                    break;
                case 5:
                    DoPlaylists();
                    CurrentMenu = 3;
                    break;
            }
            for(vector<string>::iterator iter = List.begin(); iter != List.end();
                    iter++) {
                ActiveMenu->AddOption((*iter).c_str());
            }
            Globals::Display.Update(ActiveMenu);
            return 1;
            break;
            
        case 2:
            Globals::Display.ShowHourglass();
            Selection = ActiveMenu->GetSelection();
            if(Selection > 1) {
                HttpConnection::UrlEncode(List[Selection - 2]);
                DoResults(SearchField, List[Selection - 2].c_str());
            }
            else {
                DoResults(SearchField, "");
            }
            CurrentMenu = 0;
            for(IDiter = SongList.begin();
                    IDiter != SongList.end(); IDiter++) {
                Globals::Playlist.Enqueue(this, (*IDiter).ID);
            }
            Globals::Display.RemoveTopScreen(ActiveMenu);
            return 2;
            break;
            
        case 3:
            IDiter = SongList.begin();
            /* Seems like there should be a better way to do this */
            for(int i = 0; i < (ActiveMenu->GetSelection() - 1); i++) {
                IDiter++;
            }
            DoPlaylistContents((*IDiter).ID);
            for(IDiter = SongList.begin(); IDiter != SongList.end(); IDiter++) {
                Globals::Playlist.Enqueue(this, (*IDiter).ID);
            }
            Globals::Display.RemoveTopScreen(ActiveMenu);
            CurrentMenu = 0;
            return 2;
            break;
    }
    return 0;
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
        Dec = new Mp3Decoder(ServerConn->GetDescriptor(), &PlaylistAudioOut, this);
    }
    else if(strcmp(TrackTag.Codec, "flac") == 0) {
        Dec = new FlacDecoder(ServerConn->GetDescriptor(), &PlaylistAudioOut, this);
    }
    else if(strcmp(TrackTag.Codec, "ogg") == 0) {
        Log::GetInstance()->Post(LOG_ERROR, __FILE__, __LINE__,
                "OGG format is not yet supported");
        return;
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
