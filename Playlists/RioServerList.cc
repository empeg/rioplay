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
#include "RioServerList.hh"
#include "Playlist.hh"
#include "Http.hh"
#include "Screen.hh"
#include "DisplayThread.hh"
#include "Commands.h"
#include "MenuScreen.hh"
#include "KeyCodes.h"
#include "Log.hh"
#include "MemAlloc.hh"

extern int errno;

extern DisplayThread Display;

RioServerList::RioServerList(void) {
    char SSDPRequest[] = "upnp:uuid:1D274DB0-F053-11d3-BF72-0050DA689B2F";
    struct sockaddr_in LocalAddr, BroadcastAddr;
    int SSDP;
    int One = 1;
    char TempString[256];
    FILE *fp;
    char *HttpLoc, *ColonLoc, *SlashLoc;
    
    NumEntries = 0;
    SongID = NULL;
    List = NULL;
    
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

RioServerList::~RioServerList(void) {
    for(int i = 0; i < NumEntries; i++) {
        __free(List[i]);
    }
    if(List != NULL) {
        __free(List);
    }
    if(SongID != NULL) {
        __free(SongID);
    }
}

void RioServerList::DoQuery(char *Field, char *Query) {
    HttpConnection *Http = NULL;
    char TempString[128];
    char *TempPtr;
    int i;
    FILE *fp;
    
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
    fgets(TempString, 128, fp);

    /* Free the old list */
    if(List != NULL) {
        for(i = 0; i < NumEntries; i++) {
            __free(List[i]);
        }
        __free(List);
        List = NULL;
    }
    
    /* Read query responses */
    for(i = 0; fgets(TempString, 128, fp) > 0; i++) {
        /* Check to see if we need to expand the array */
        if((i % 10) == 0) {
            List = (char **) __realloc(List, sizeof(char *) * (i + 10));
        }
        
        /* fgets() does not remove trailing '\n', so 
           we need to take care of it */
        if(TempString[strlen(TempString) - 1] == '\n') {
            TempString[strlen(TempString) - 1] = '\0';
        }
        
        /* Get start pointer of actual title string */
        TempPtr = strstr(TempString, ":") + 1;
        
        /* Allocate space for the title string */
        List[i] = (char *) __malloc(sizeof(char) * (strlen(TempPtr) + 1));
        
        /* Assign */
        strcpy(List[i], TempPtr);
    }
    NumEntries = i;
    if(NumEntries > 0) {
        Position = 1;
    }
    else {
        Position = 0;
    }

    /* Close file descriptor and socket */
    delete Http;
}

void RioServerList::DoResults(char *Field, char *Query) {
    HttpConnection *Http = NULL;
    char TempString[128];
    int TempSongID;
    int i;
    FILE *fp;

    NumSongIDEntries = 0;
    SongIDPosition = 0;
    
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
    
    /* Free SongID list */
    if(SongID != NULL) {
        __free(SongID);
        SongID = NULL;
    }
    
    /* Read the Song ID */
    for(i = 0; fgets(TempString, 128, fp) > 0; i++) {
        sscanf(TempString, "%x=", &TempSongID);
        /* Check to see if we need to expand the array */
        if((i % 10) == 0) {
            SongID = (int *) __realloc(SongID, sizeof(int) * (i + 10));
        }
        SongID[i] = TempSongID;
    }
    NumSongIDEntries = i;
    SongIDPosition = 1;

    /* Close file descriptor and socket */
    delete Http;
}

void RioServerList::DoPlaylists(void) {
    HttpConnection *Http = NULL;
    char TempString[128];
    char *TempPtr;
    int i;
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
    
    /* Free the old list */
    if(List != NULL) {
        for(i = 0; i < NumEntries; i++) {
            __free(List[i]);
        }
        __free(List);
        List = NULL;
    }
    if(SongID != NULL) {
        __free(SongID);
        SongID = NULL;
    }
    
    /* Read query responses */
    for(i = 0; fgets(TempString, 128, fp) > 0; i++) {
        /* Check to see if we need to expand the array */
        if((i % 10) == 0) {
            List = (char **) __realloc(List, sizeof(char *) * (i + 10));
            SongID = (int *) __realloc(SongID, sizeof(int) * (i + 10));
        }
        
        /* fgets() does not remove trailing '\n', so 
           we need to take care of it */
        if(TempString[strlen(TempString) - 1] == '\n') {
            TempString[strlen(TempString) - 1] = '\0';
        }
        
        /* Get start pointer of actual title string */
        TempPtr = strstr(TempString, "=") + 2;
        
        /* Allocate space for the title string */
        List[i] = (char *) __malloc(sizeof(char) * (strlen(TempPtr) + 1));
        
        /* Assign */
        strcpy(List[i], TempPtr);
        SongID[i] = strtol(TempString, NULL, 16);
    }
    NumEntries = i;
    NumSongIDEntries = i;
    if(NumEntries > 0) {
        Position = 1;
        SongIDPosition = 1;
    }
    else {
        Position = 0;
        SongIDPosition = 0;
    }

    /* Close file descriptor and socket */
    delete Http;
}

void RioServerList::DoPlaylistContents(int ID) {
    HttpConnection *Http = NULL;
    char TempString[128];
    int i;
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
    if(SongID != NULL) {
        __free(SongID);
        SongID = NULL;
    }
    
    /* Read the Song ID */
    for(i = 0; fgets(TempString, 128, fp) > 0; i++) {
        sscanf(TempString, "%x=", &TempSongID);
        /* Check to see if we need to expand the array */
        if((i % 10) == 0) {
            SongID = (int *) __realloc(SongID, sizeof(int) * (i + 10));
        }
        SongID[i] = TempSongID;
    }
    NumSongIDEntries = i;
    SongIDPosition = 1;

    /* Close file descriptor and socket */
    delete Http;
    
    /* Return the result */
    return;
}

Tag RioServerList::GetTag(int EntryNumber) {
    HttpConnection *Http = NULL;
    int QueryFD;
    char TempString[128];
    Tag ReturnVal;
    char Data[128];
    unsigned char Key;
    unsigned char Size;

    /* Set to zeros to be safe */
    bzero(&ReturnVal, sizeof(ReturnVal));
    
    if(EntryNumber <= 0) {
        return ReturnVal;
    }
    
    /* Open a connection to the Rio HTTP Server */    
    sprintf(TempString, "http://%s:%d/tags/%x", Server, Port, SongID[EntryNumber - 1]);
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
                
            default:
                break;
        }
    }

    /* Close file descriptor and socket */
    delete Http;    
    
    /* Return the array of results */
    return ReturnVal;
}

char *RioServerList::GetFilename(char *Filename, int EntryNumber) {
    if(EntryNumber <= 0) {
        return NULL;
    }
    sprintf(Filename, "http://%s:%d/content/%x", Server, Port, SongID[EntryNumber - 1]);
    return Filename;
}

int RioServerList::CommandHandler(unsigned int Keycode, MenuScreen *ActiveMenu) {
    static int CurrentMenu = 0;
    int Selection;
    
    if((Keycode == PANEL_MENU) || (Keycode == REMOTE_MENU)) {
        Display.RemoveTopScreen(ActiveMenu);
        CurrentMenu = 0;
        return 0;
    }
    else if((Keycode == PANEL_WHEEL_CW) || (Keycode == REMOTE_DOWN) || (Keycode == REMOTE_DOWN_REPEAT)) {
        ActiveMenu->Advance();
        Display.Update(ActiveMenu);
        return 1;
    }
    else if((Keycode == PANEL_WHEEL_CCW) || (Keycode == REMOTE_UP) || (Keycode == REMOTE_UP_REPEAT)) {
        ActiveMenu->Reverse();
        Display.Update(ActiveMenu);
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
            Display.Update(ActiveMenu);
            return 1;
            break;
            
        case 1:
            Selection = ActiveMenu->GetSelection();
            ActiveMenu->ClearOptions();
            ActiveMenu->SetTitle("Select Artist");
            CurrentMenu = 2;
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
            for(int i = 0; i < NumEntries; i++ ) {
                ActiveMenu->AddOption(List[i]);
            }
            Display.Update(ActiveMenu);
            return 1;
            break;
            
        case 2:
            Selection = ActiveMenu->GetSelection();
            if(Selection > 1) {
                if(Selection == 2) {
                    for(int i = 1; i < NumEntries; i++) {
                        __free(List[i]);
                    }
                    NumEntries = 1;
                }
                else {
                    List[0] = (char *) __realloc(List[0], strlen(List[Selection - 2]) + 1);
                    strcpy(List[0], List[Selection - 2]);
                    for(int i = 1; i < NumEntries; i++) {
                        __free(List[i]);
                    }
                    NumEntries = 1;
                }
            }
            DoResults(SearchField, HttpConnection::UrlEncode(&List[0]));
            Display.RemoveTopScreen(ActiveMenu);
            CurrentMenu = 0;
            return 2;
            break;
            
        case 3:
            Selection = ActiveMenu->GetSelection();
            DoPlaylistContents(SongID[Selection - 1]);
            NumEntries = 1;
            Display.RemoveTopScreen(ActiveMenu);
            CurrentMenu = 0;
            return 2;
            break;
    }
    return 0;
}

void RioServerList::Advance(void) {
    int OldPosition;
    
    SongIDPosition++;
    if(SongIDPosition > NumSongIDEntries) {
        /* We've completed this sub-list, move on to the next sub-list */
        OldPosition = Position;
        Playlist::Advance();
        
        /* Build a new sub-list of SongIDs */
        if(OldPosition != Position) {
            DoResults(SearchField, HttpConnection::UrlEncode(&List[Position - 1]));
        }
        SongIDPosition = 1;
    }
}        

void RioServerList::Reverse(void) {
    int OldPosition;
    
    SongIDPosition--;
    if(SongIDPosition < 1) {
        /* We've completed this sub-list, move on to the previous sub-list */
        OldPosition = Position;
        Playlist::Reverse();

        /* Build a new sub-list of SongIDs */
        if(OldPosition != Position) {
            DoResults(SearchField, HttpConnection::UrlEncode(&List[Position - 1]));
        }
        SongIDPosition = NumSongIDEntries;
    }
}

int RioServerList::GetPosition(void) {
    return SongIDPosition;
}
