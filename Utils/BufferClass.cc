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
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "BufferClass.hh"
#include "Log.hh"
#include "MemAlloc.hh"

#define MIN_BUFFER_SIZE 8192

extern int errno;

BufferClass::BufferClass(int inFileD, int inBufSize) {
    /* Initialize class members */
    BytesInBuffer = 0;
    ReaderThreadActive = false;
    BufferFull = false;
    
    /* Save the passed in arguments */
    FileD = inFileD;
    BufferSize = inBufSize;
    if(BufferSize < (MIN_BUFFER_SIZE * 2)) {
        BufferSize = (MIN_BUFFER_SIZE * 2);
    }
    
    /* Allocate buffer */
    Buf = new char[BufferSize];
    if(Buf == NULL) {
        Log::GetInstance()->Post(LOG_FATAL, __FILE__, __LINE__,
                "new() failed");
        return;
    }
    
    /* Start the reader thread */
    Start();
    
    /* Wait for signal which indicates that reader thread is running */
    pthread_mutex_lock(&ClassMutex);
    pthread_cond_wait(&ClassCondition, &ClassMutex);
    pthread_mutex_unlock(&ClassMutex);
}

BufferClass::~BufferClass(void) {
    /* Cancel the reader thread */
    if(pthread_cancel(ThreadHandle) != ESRCH) {
        /* Wait until reader thread is done */
        pthread_join(ThreadHandle, NULL);
    }
    
    /* Deallocate buffer */
    delete [] Buf;
}

/* Non blocking read from the buffer */
int BufferClass::ReadNB(void *BufOut, int NumBytes) {
    /* Lock the member variables */
    pthread_mutex_lock(&ClassMutex);
    
    if((ReaderThreadActive == false) && (BytesInBuffer == 0)) {
        /* Descriptor has been closed, no more data is coming */
        pthread_mutex_unlock(&ClassMutex);
        return -1;
    }
    
    /* Check to see if caller requested more bytes than are available */
    if(NumBytes > BytesInBuffer) {
        NumBytes = BytesInBuffer;
    }
    
    /* Copy requested data into caller-supplied buffer */
    memcpy(BufOut, Buf, NumBytes);
    
    /* Move data that's still in our buffer to the beginning */
    BytesInBuffer -= NumBytes;
    memmove(Buf, Buf + NumBytes, BytesInBuffer);

    /* Unlock member variables */
    pthread_mutex_unlock(&ClassMutex);
    
    /* Signal reader thread that Buffer has more space available */
    pthread_cond_signal(&ClassCondition);
    
    /* Indicate to caller how many bytes were actually provided */
    return NumBytes;
}

void *BufferClass::ThreadMain(void *arg) {
    /* This is the reader thread.  It is responsible for reading from the
       file descriptor and filling the Buffer member variable with 
       up to BufferSize bytes and keeping it as full as possible */
    char LocalBuffer[MIN_BUFFER_SIZE];
    int BytesRead;
    
    ReaderThreadActive = true;
    pthread_cond_signal(&ClassCondition);
    while(1) {
        BytesRead = read(FileD, LocalBuffer, MIN_BUFFER_SIZE);
        if(BytesRead <= 0) {
            Log::GetInstance()->Post(LOG_ERROR, __FILE__, __LINE__,
                    "read() failed while refilling buffer: %s", strerror(errno));
            
            /* Indicate the reader thread has ended */
            pthread_mutex_lock(&ClassMutex);
            ReaderThreadActive = false;
            pthread_mutex_unlock(&ClassMutex);
            
            pthread_exit(0);
        }
        else if(BytesRead == 0) {
            /* Nothing to read.  Sleep for 1/10 second to prevent constantly
               calling read() when no data is available */
            usleep(100000);
        }
        else {
            /* Lock member variables */
            pthread_mutex_lock(&ClassMutex);

            while(BufferSize - BytesInBuffer < BytesRead) {
                /* Wait until there's enough space in Buffer for the bytes we
                   just read */
                
                /* Indicate full buffer */
                BufferFull = true;
                
                /* Wait for signal indicating space is available in buffer */
                pthread_cond_wait(&ClassCondition, &ClassMutex);
            }

            /* Copy data we just read() into Buffer */
            memcpy(Buf + BytesInBuffer, LocalBuffer, BytesRead);
            BytesInBuffer += BytesRead;

            /* Unlock member variables */
            pthread_mutex_unlock(&ClassMutex);
        }
    }
}

void BufferClass::Prebuffer(void) {
    /* Wait for signal indicating buffer is full */
    pthread_mutex_lock(&ClassMutex);
    while(BufferFull == false) {
        pthread_mutex_unlock(&ClassMutex);
        usleep(10000);
        pthread_mutex_lock(&ClassMutex);
    }
    pthread_mutex_unlock(&ClassMutex);
}    
