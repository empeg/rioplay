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

#ifndef BUFFERCLASS_HH
#define BUFFERCLASS_HH

#include "Thread.hh"

class BufferClass : public Thread {
public:
    BufferClass(int inFileD, int inBufSize);
    ~BufferClass(void);
    int ReadNB(void *BufOut, int NumBytes);
    void *ThreadMain(void *arg);
    void Prebuffer(void);
    
private:
    int FileD; /* File descriptor to read from */
    char *Buf;
    int BufferSize;
    int BytesInBuffer;
    bool ReaderThreadActive;
    bool BufferFull;
};

#endif /* #ifndef BUFFERCLASS_HH */
