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
#include "Http.h"

/* Returns metadata frequency if metadata is detected, otherwise 0 */
int HttpSkipHeader(FILE *fp) {
    int GotCRLF = 0;
    char TempString[512];
    char *TempPtr;
    int ReturnVal = 0;
    
    /* Read off HTTP header ("\r\n\r\n" indicates end of header) */
    while(GotCRLF == 0) {
        fgets(TempString, 512, fp);
        if((TempString[0] == '\r') && (TempString[1] == '\n')) {
            GotCRLF = 1;
        }
        
        if((TempPtr = strstr(TempString, "icy-metaint"))) {
            TempPtr += 12;
            ReturnVal = atoi(TempPtr);
            printf("Http: Metadata Frequency: %d\n", ReturnVal);
        }
    }
    
    return ReturnVal;
}

FILE *HttpConnect(char *Host, unsigned short Port) {
    int fd;
    FILE *fp;
    struct sockaddr_in ServAddr;
    struct hostent *HostEntry;
    
    bzero((char *) &ServAddr, sizeof(ServAddr));
    ServAddr.sin_family = AF_INET;
    ServAddr.sin_port = htons(Port);

    /* Do DNS lookup */
    if((HostEntry = gethostbyname(Host))) {
        ServAddr.sin_addr.s_addr = 
                ((struct in_addr *)(HostEntry->h_addr))->s_addr;
    }
    else {
        printf("Http: Error: could not find host '%s'\n", Host);
        return NULL;
    }

    /* Open a TCP socket */
    if((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Http: Socket");
        return NULL;
    }

    /* Connect to the server */
    if(connect(fd, (struct sockaddr *) &ServAddr, sizeof(ServAddr)) < 0) {
        perror("Http: Connect");
        return NULL;
    }

    fp = fdopen(fd, "r+");
    
    return fp;
}

void HttpParseUrl(char *Url, char *Host, unsigned short *Port, char *Path) {
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
        *Port = atoi(TempPtr + 1);
    }
    else {
        /* No port in URL, default to 80 */
        *Port = 80;
    }
}

char *HttpUrlEncode(char **string) {
    char Temp[256];
    unsigned int Orig, New;

    for(Orig = 0, New = 0; Orig < (strlen(*string) + 1); Orig++) {
         if((*string)[Orig] != ' ') {
             Temp[New] = (*string)[Orig];
             New++;
         }
         else {
             sprintf(&(Temp[New]), "%%20");
             New += 3;
         }
    }

    *string = (char *) realloc(*string, (sizeof(char) * (strlen(Temp) + 1)));
    
    strcpy(*string, Temp);
   
    return *string;
}
