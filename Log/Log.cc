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
#include <pthread.h>
#include <stdarg.h>
#include <string.h>
#include "Log.hh"
#include "MemAlloc.hh"

#define LOG_MAX_EVENTS 50 /* This should probably be a multiple of 5 */

/* Set static member "Instance" to NULL */
Log *Log::Instance = NULL;

Log::Log(void) {
    NumEvents = 0;
    EventSeverity = NULL;
    InFile = NULL;
    AtLine = NULL;
    MessageLogged = NULL;
    pthread_mutex_init(&LogMutex, NULL);
    if(Instance == NULL) {
        Instance = this;
    }
}

Log::~Log(void) {
    for(int i = 0; i < NumEvents; i++) {
        __free(InFile[i]);
        __free(MessageLogged[i]);
    }
    __free(EventSeverity);
    __free(InFile);
    __free(AtLine);
    __free(MessageLogged);
}

Log *Log::GetInstance(void) {
    if(Instance == NULL) {
        Instance = new Log;
    }
    
    return Instance;
}

void Log::Post(int Severity, char *File, int Line, char *Message, ...) {
    int TempStringLength;
    char *TempString;
    va_list ArgList;
    
    /* Lock the log */
    pthread_mutex_lock(&LogMutex);
    
    /* If log size is divisible by 5, add 5 empty log entries */
    /* Keep log to a maximum of LOG_MAX_EVENTS */
    if((NumEvents % 5) == 0) {
        if(NumEvents + 5 <= LOG_MAX_EVENTS) {
            EventSeverity = (int *) __realloc(EventSeverity, sizeof(int) * (NumEvents + 5));
            InFile = (char **) __realloc(InFile, sizeof(char *) * (NumEvents + 5));
            AtLine = (int *) __realloc(AtLine, sizeof(int) * (NumEvents + 5));
            MessageLogged = (char **) __realloc(MessageLogged, sizeof(char *) * (NumEvents + 5));
        }
        else {
            /* Delete 5 oldest entries from the log */
            for(int i = 0; i < 5; i++) {
                __free(InFile[i]);
                __free(MessageLogged[i]);
            }
            memmove(EventSeverity, &EventSeverity[5], sizeof(int) * (NumEvents - 5));
            memmove(InFile, &InFile[5], sizeof(char *) * (NumEvents - 5));
            memmove(AtLine, &AtLine[5], sizeof(int) * (NumEvents - 5));
            memmove(MessageLogged, &MessageLogged[5], sizeof(char *) * (NumEvents - 5));
            NumEvents -= 5;
        }
    }
    
    /* Copy information into log */
    EventSeverity[NumEvents] = Severity;
    AtLine[NumEvents] = Line;
    InFile[NumEvents] = (char *) __malloc(sizeof(char) * (strlen(File) + 1));
    strcpy(InFile[NumEvents], File);
    
    /* Build message string */
    va_start(ArgList, Message);
    TempString = (char*) __malloc(sizeof(char)*1024); 
    TempStringLength = vsprintf(TempString, Message, ArgList) + 1;
    va_end(ArgList);

    TempString = (char *) __realloc(TempString, TempStringLength);
    MessageLogged[NumEvents] = TempString;

    printf("%s, Line %d, %s\n", File, Line, TempString);
        
    NumEvents++;
    
    /* Unlock the log */
    pthread_mutex_unlock(&LogMutex);
}

int Log::GetNumEvents(void) {
    return NumEvents;
}

int Log::GetSeverity(int Event) {
    /* Change from 1 based to 0 based */
    Event--;
    
    /* Check for out of range event number */
    if((Event >= NumEvents) || (Event < 0)) {
        return -1;
    }
    
    return EventSeverity[Event];
}

char *Log::GetFile(int Event) {
    /* Change from 1 based to 0 based */
    Event--;
    
    /* Check for out of range event number */
    if((Event >= NumEvents) || (Event < 0)) {
        return NULL;
    }
    
    return InFile[Event];
}

int Log::GetLine(int Event) {
    /* Change from 1 based to 0 based */
    Event--;
    
    /* Check for out of range event number */
    if((Event >= NumEvents) || (Event < 0)) {
        return -1;
    }
    
    return AtLine[Event];
}

char *Log::GetMessage(int Event) {
    /* Change from 1 based to 0 based */
    Event--;
    
    /* Check for out of range event number */
    if((Event >= NumEvents) || (Event < 0)) {
        return NULL;
    }
    
    return MessageLogged[Event];
}
