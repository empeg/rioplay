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
#include "RioServerList.hh"
#include "Playlist.hh"
#include "Http.h"
#include "Screen.hh"
#include "DisplayThread.hh"
#include "Commands.h"
#include "MenuScreen.hh"
#include "KeyCodes.h"

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
        printf("RioServerList: Could not get socket\n");
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
        printf("RioServerList: Could not bind\n");
        return;
    }

    if(setsockopt(SSDP, SOL_SOCKET, SO_BROADCAST, &One, sizeof(One)) < 0) {
        perror("RioServerList: Setsockopt");
    }
    
    if(sendto(SSDP, SSDPRequest, strlen(SSDPRequest), 0,
            (struct sockaddr *) &BroadcastAddr, sizeof(BroadcastAddr)) < 0) {
        perror("RioServerList: Sendto");
    }

    fp = fdopen(SSDP, "r");
    
    fgets(TempString, 256, fp);
    HttpLoc = strstr(TempString, "http://") + 7;
    ColonLoc = strstr(HttpLoc, ":") + 1;
    SlashLoc = strstr(ColonLoc, "/");
    memcpy(Server, HttpLoc, ColonLoc - HttpLoc - 1);
    Server[ColonLoc - HttpLoc - 1] = '\0';
    Port = atoi(ColonLoc);
    
    close(SSDP);
}

RioServerList::~RioServerList(void) {
    for(int i = 0; i < NumEntries; i++) {
        free(List[i]);
    }
    if(List != NULL) {
        free(List);
    }
}

void RioServerList::DoQuery(char *Field, char *Query) {
    FILE *QueryFP;
    char TempString[128];
    char *TempPtr;
    int i;
    
    strcpy(SearchField, Field);
    
    /* Open a connection to the Rio HTTP Server */    
    if((QueryFP = HttpConnect(Server, Port)) == NULL) {
        return;
    }
    
    /* Send HTTP request */
    sprintf(TempString, "GET /query?%s=%s HTTP/1.0\r\n\r\n", Field, Query);
    fwrite(TempString, 1, strlen(TempString), QueryFP);

    /* Find end of HTTP Header */
    HttpSkipHeader(QueryFP);
    
    /* Throw away "matches=" response */
    fgets(TempString, 128, QueryFP);

    /* Free the old list */
    if(List != NULL) {
        for(i = 0; i < NumEntries; i++) {
            free(List[i]);
        }
        free(List);
        List = NULL;
    }
    
    /* Read query responses */
    for(i = 0; fgets(TempString, 128, QueryFP) != NULL; i++) {
        /* Check to see if we need to expand the array */
        if((i % 10) == 0) {
            List = (char **) realloc(List, sizeof(char *) * (i + 10));
        }
        
        /* fgets() does not remove trailing '\n', so 
           we need to take care of it */
        if(TempString[strlen(TempString) - 1] == '\n') {
            TempString[strlen(TempString) - 1] = '\0';
        }
        
        /* Get start pointer of actual title string */
        TempPtr = strstr(TempString, ":") + 1;
        
        /* Allocate space for the title string */
        List[i] = (char *) malloc(sizeof(char) * (strlen(TempPtr) + 1));
        
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
    fclose(QueryFP);
}

void RioServerList::DoResults(char *Field, char *Query) {
    FILE *QueryFP;
    char TempString[128];
    int TempSongID;
    int i;

    NumSongIDEntries = 0;
    SongIDPosition = 0;
    
    /* Open a connection to the Rio HTTP Server */    
    if((QueryFP = HttpConnect(Server, Port)) == NULL) {
        printf("HttpConnect failed!\n");
        return;
    }

    /* Send HTTP request */
    sprintf(TempString, "GET /results?%s=%s&_extended=1 HTTP/1.0\r\n\r\n",
            Field, Query);
    fwrite(TempString, 1, strlen(TempString), QueryFP);

    /* Find end of HTTP Header */
    HttpSkipHeader(QueryFP);
    
    /* Free SongID list */
    if(SongID != NULL) {
        free(SongID);
        SongID = NULL;
    }
    
    /* Read the Song ID */
    for(i = 0; fgets(TempString, 128, QueryFP) != NULL; i++) {
        sscanf(TempString, "%x=", &TempSongID);
        /* Check to see if we need to expand the array */
        if((i % 10) == 0) {
            SongID = (int *) realloc(SongID, sizeof(int) * (i + 10));
        }
        SongID[i] = TempSongID;
    }
    NumSongIDEntries = i;
    SongIDPosition = 1;

    /* Close file descriptor and socket */
    fclose(QueryFP);
    
    /* Return the result */
    return;
}

void RioServerList::DoPlaylists(void) {
    FILE *QueryFP;
    char TempString[128];
    char *TempPtr;
    int i;
    
    /* Open a connection to the Rio HTTP Server */    
    if((QueryFP = HttpConnect(Server, Port)) == NULL) {
        return;
    }
    
    /* Send HTTP request */
    sprintf(TempString, "GET /content/100?_extended=1 HTTP/1.0\r\n\r\n");
    fwrite(TempString, 1, strlen(TempString), QueryFP);

    /* Find end of HTTP Header */
    HttpSkipHeader(QueryFP);
    
    /* Free the old list */
    if(List != NULL) {
        for(i = 0; i < NumEntries; i++) {
            free(List[i]);
        }
        free(List);
        List = NULL;
    }
    if(SongID != NULL) {
        free(SongID);
        SongID = NULL;
    }
    
    /* Read query responses */
    for(i = 0; fgets(TempString, 128, QueryFP) != NULL; i++) {
        /* Check to see if we need to expand the array */
        if((i % 10) == 0) {
            List = (char **) realloc(List, sizeof(char *) * (i + 10));
            SongID = (int *) realloc(SongID, sizeof(int) * (i + 10));
        }
        
        /* fgets() does not remove trailing '\n', so 
           we need to take care of it */
        if(TempString[strlen(TempString) - 1] == '\n') {
            TempString[strlen(TempString) - 1] = '\0';
        }
        
        /* Get start pointer of actual title string */
        TempPtr = strstr(TempString, "=") + 2;
        
        /* Allocate space for the title string */
        List[i] = (char *) malloc(sizeof(char) * (strlen(TempPtr) + 1));
        
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
    fclose(QueryFP);
}

void RioServerList::DoPlaylistContents(int ID) {
    FILE *QueryFP;
    char TempString[128];
    int i;
    int TempSongID;
    
    /* Open a connection to the Rio HTTP Server */    
    if((QueryFP = HttpConnect(Server, Port)) == NULL) {
        return;
    }
    
    /* Send HTTP request */
    sprintf(TempString, "GET /content/%x?_extended=1 HTTP/1.0\r\n\r\n", ID);
    fwrite(TempString, 1, strlen(TempString), QueryFP);

    /* Find end of HTTP Header */
    HttpSkipHeader(QueryFP);
    
    /* Free SongID list */
    if(SongID != NULL) {
        free(SongID);
        SongID = NULL;
    }
    
    /* Read the Song ID */
    for(i = 0; fgets(TempString, 128, QueryFP) != NULL; i++) {
        sscanf(TempString, "%x=", &TempSongID);
        /* Check to see if we need to expand the array */
        if((i % 10) == 0) {
            SongID = (int *) realloc(SongID, sizeof(int) * (i + 10));
        }
        SongID[i] = TempSongID;
    }
    NumSongIDEntries = i;
    SongIDPosition = 1;

    /* Close file descriptor and socket */
    fclose(QueryFP);
    
    /* Return the result */
    return;
}

Tag RioServerList::GetTag(int EntryNumber) {
    FILE *QueryFP;
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
    if((QueryFP = HttpConnect(Server, Port)) == NULL) {
        return ReturnVal;
    }

    /* Send HTTP request */
    sprintf(TempString, "GET /tags/%x HTTP/1.0\r\n\r\n", SongID[EntryNumber - 1]);
    fwrite(TempString, 1, strlen(TempString), QueryFP);

    /* Find end of HTTP Header */
    HttpSkipHeader(QueryFP);
    
    /* Parse tag data */
    while(fread(&Key, 1, 1, QueryFP)) {
        fread(&Size, 1, 1, QueryFP);
        fread(Data, 1, Size, QueryFP);
        
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
    
    fclose(QueryFP);
    
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
                        free(List[i]);
                    }
                    NumEntries = 1;
                }
                else {
                    List[0] = (char *) realloc(List[0], strlen(List[Selection - 2]) + 1);
                    strcpy(List[0], List[Selection - 2]);
                    for(int i = 1; i < NumEntries; i++) {
                        free(List[i]);
                    }
                    NumEntries = 1;
                }
            }
            DoResults(SearchField, HttpUrlEncode(&List[0]));
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
            DoResults(SearchField, HttpUrlEncode(&List[Position - 1]));
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
            DoResults(SearchField, HttpUrlEncode(&List[Position - 1]));
        }
        SongIDPosition = NumSongIDEntries;
    }
}

int RioServerList::GetPosition(void) {
    return SongIDPosition;
}
