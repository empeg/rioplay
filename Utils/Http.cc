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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <string>
#include "Http.hh"
#include "Log.hh"
#include "Player.h"
#include "MemAlloc.hh"

extern int errno;

HttpConnection::HttpConnection(char *Url) {
    /* Initialize member variables */
    Descriptor = -1;
    FilePointer = NULL;
    
    /* Copy passed in arguments */
    ParseUrl(Url);
}

HttpConnection::~HttpConnection(void) {
    if(Descriptor != -1) {
        if(FilePointer != NULL) {
            fclose(FilePointer);
        }
        else {
            close(Descriptor);
        }
    }
    else {
        Log::GetInstance()->Post(LOG_ERROR, __FILE__, __LINE__,
                "HttpConnection instance had invalid descriptor");
    }
}

int HttpConnection::Connect(void) {
    struct sockaddr_in ServAddr;
    struct hostent HostEntry, *HEReturn;
    char TempString[256];
    char HostBuffer[512];
    int HostError;
    
    bzero((char *) &ServAddr, sizeof(ServAddr));
    ServAddr.sin_family = AF_INET;
    ServAddr.sin_port = htons(Port);

    if(Descriptor != -1) {
        Log::GetInstance()->Post(LOG_ERROR, __FILE__, __LINE__,
                "HttpConnection is already connected");
    }
    
    /* Do DNS lookup */
    if(gethostbyname_r(Host, &HostEntry, HostBuffer, 512, &HEReturn, &HostError) == 0) {
        ServAddr.sin_addr.s_addr = 
                ((struct in_addr *)(HostEntry.h_addr))->s_addr;
    }
    else {
        Log::GetInstance()->Post(LOG_ERROR, __FILE__, __LINE__,
                "gethostbyname_r() failed on host %s", Host);
        return -1;
    }

    /* Open a TCP socket */
    if((Descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        Log::GetInstance()->Post(LOG_ERROR, __FILE__, __LINE__,
                "socket() failed: %s", strerror(errno));
        return -1;
    }

    /* Connect to the server */
    if(connect(Descriptor, (struct sockaddr *) &ServAddr, sizeof(ServAddr)) < 0) {
        Log::GetInstance()->Post(LOG_ERROR, __FILE__, __LINE__,
                "connect() failed: %s", strerror(errno));
        fflush(stdout);
        return -1;
    }

    /* Send HTTP request */
    sprintf(TempString,
            "GET /%s HTTP/1.0\r\nHost: %s\r\nUser-Agent: RioPlay/%s\r\nicy-metadata:1\r\n\r\n",
            Path, Host, PLAYER_VER);
    write(Descriptor, TempString, strlen(TempString));
        
    return Descriptor;
}

void HttpConnection::ParseUrl(char *Url) {
    char *TempPtr;

    /* Skip over "http://" */
    strcpy(Host, Url + 7);
    
    /* Find start of path portion of URL */
    TempPtr = strstr(Host, "/");
    
    if(TempPtr != NULL) {
        /* Chop off path from hostname */
        TempPtr[0] = '\0';

        /* Copy path to Path */
        strcpy(Path, TempPtr + 1);
    }
    else {
        /* No path in URL, terminate it correctly */
        Path[0] = '\0';
    }

    /* Check for port number leftover in Host */
    TempPtr = strstr(Host, ":");
    if(TempPtr != NULL) {
        /* Chop off port from hostname */
        TempPtr[0] = '\0';

        /* Assign Port */
        Port = atoi(TempPtr + 1);
    }
    else {
        /* No port in URL, default to 80 */
        Port = 80;
    }
}

/* Returns metadata frequency if metadata is detected, otherwise 0 */
int HttpConnection::SkipHeader(void) {
    int GotCRLF = 0;
    char TempString[512];
    char *TempPtr;
    int ReturnVal = 0;
    
    /* Read off HTTP header ("\r\n\r\n" indicates end of header) */
    while(GotCRLF == 0) {
        GetString(TempString, 512);
        if((TempString[0] == '\r') && (TempString[1] == '\n')) {
            GotCRLF = 1;
        }
        
        if((TempPtr = strstr(TempString, "icy-metaint"))) {
            TempPtr += 12;
            ReturnVal = atoi(TempPtr);
            Log::GetInstance()->Post(LOG_INFO, __FILE__, __LINE__,
                    "Found Metadata Frequency: %d", ReturnVal);
        }
    }
    
    return ReturnVal;
}

void HttpConnection::UrlEncode(string &Str) {
    string::size_type Pos;
    string HttpSpace("%20");
    
    while((Pos = Str.find(" ", 0)) != string::npos) {
        Str.replace(Pos, 1, HttpSpace);
    }
}

int HttpConnection::GetString(char *buf, int bufsize) {
    int Length = 0, Temp;
    
    if((Temp = read(Descriptor, buf + Length, 1)) <= 0) {
        return Temp;
    }
    Length++;
    while(buf[Length - 1] != '\n') {
        if((Temp = read(Descriptor, buf + Length, 1)) <= 0) {
            return Temp;
        }
        Length++;
    }
    buf[Length] = '\0';
    
    return Length;
}
