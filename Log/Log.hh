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

#ifndef LOG_HH
#define LOG_HH

#include <pthread.h>

#define LOG_INFO     1
#define LOG_WARNING  2
#define LOG_ERROR    3
#define LOG_FATAL    4

class Log {
public:
    Log(void);
    ~Log(void);
    static Log *GetInstance(void);
    void Post(int Severity, char *File, int Line, char *Message, ...);
    int GetNumEvents(void);
    int GetSeverity(int Event);
    char *GetFile(int Event);
    int GetLine(int Event);
    char *GetMessage(int Event);
    
private:
    static Log *Instance;
    int NumEvents;
    int *EventSeverity;
    char **InFile;
    int *AtLine;
    char **MessageLogged;
    pthread_mutex_t LogMutex;
};

#endif /* #ifndef LOG_HH */
