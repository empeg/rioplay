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

#ifndef HTTP_HH
#define HTTP_HH

#include <stdio.h>
#include <string>

class HttpConnection {
public:
    HttpConnection(char *Url);
    ~HttpConnection(void);
    int Connect(void);
    int SkipHeader(void);
    int GetString(char *buf, int bufsize);
    int GetDescriptor(void);
    FILE *GetFilePointer(void);
    static void UrlEncode(string &Str);
    
private:
    void ParseUrl(char *Url);
    int Descriptor;
    FILE *FilePointer;
    char Host[1024];
    unsigned int Port;
    char Path[1024];
};

inline int HttpConnection::GetDescriptor(void) {
    return Descriptor;
}

inline FILE *HttpConnection::GetFilePointer(void) {
    FilePointer = fdopen(Descriptor, "r+");
    
    return FilePointer;
}

#endif /* #ifndef HTTP_HH */
