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
#define DONT_DEFINE_NEW
#include "MemAlloc.hh"

MemAlloc *MemAlloc::Instance = NULL;

MemAlloc::MemAlloc(void) {
    pthread_mutex_init(&ClassMutex, NULL);
    
    for(int i = 0; i < MemAllocTableSize; i++) {
        Addr[i] = NULL;
        Size[i] = 0;
        Line[i] = 0;
    }
    Index = 0;
    
    Instance = this;
}

MemAlloc::~MemAlloc(void) {
    printf("### Checking for unfreed memory...\n");
    for(int i = 0; i < MemAllocTableSize; i++) {
        if(Size[i] != 0) {
            printf("--- Memory not freed\n");
            printf("    Allocated at: %s, line %d\n", Filename[i].c_str(), Line[i]);
        }
    }
    printf("### Done.\n");
}

MemAlloc *MemAlloc::GetInstance(void) {
    return Instance;
}

void *MemAlloc::Malloc(size_t size, char *FromFile, int FromLine) {
    void *Temp;
    
    pthread_mutex_lock(&ClassMutex);
    
    Addr[Index] = malloc(size + 4);
    Size[Index] = size;
    Filename[Index] = string(FromFile);
    Line[Index] = FromLine;
    Temp = Addr[Index];
    ((char *) Temp)[size] = 0xde;
    ((char *) Temp)[size + 1] = 0xad;
    ((char *) Temp)[size + 2] = 0xbe;
    ((char *) Temp)[size + 3] = 0xef;
    
    Index++;
    if(Index >= MemAllocTableSize) {
        Index = 0;
    }
    
    pthread_mutex_unlock(&ClassMutex);
    
    return Temp;
}

void *MemAlloc::Realloc(void *ptr, size_t size, char *FromFile, int FromLine) {
    if(ptr == NULL) {
        return Malloc(size, FromFile, FromLine);
    }
    
    void *ReturnVal = NULL;
    
    pthread_mutex_lock(&ClassMutex);
    
    for(int i = 0; i < MemAllocTableSize; i++) {
        if((Addr[i] == ptr) && (Size[i] > 0)) {
            if(  (((char *) Addr[i])[Size[i]] != 0xde) ||
                 (((char *) Addr[i])[Size[i] + 1] != 0xad) ||   
                 (((char *) Addr[i])[Size[i] + 2] != 0xbe) ||
                 (((char *) Addr[i])[Size[i] + 3] != 0xef) ) {
                printf("---Overwrote end of buffer!  Realloced at %s, line %d\n", FromFile, FromLine);
                printf("   Key = 0x%02x%02x%02x%02x\n", ((char *) Addr[i])[Size[i]],
                        ((char *) Addr[i])[Size[i] + 1], ((char *) Addr[i])[Size[i] + 2],
                        ((char *) Addr[i])[Size[i] + 3]);
            }
            Addr[i] = realloc(ptr, size + 4);
            Size[i] = size;
            Filename[i] = string(FromFile);
            Line[i] = FromLine;
            
           ((char *) Addr[i])[size] = 0xde;
           ((char *) Addr[i])[size + 1] = 0xad;
           ((char *) Addr[i])[size + 2] = 0xbe;
           ((char *) Addr[i])[size + 3] = 0xef;
            
            ReturnVal = Addr[i];
        }
    }
    
    if(ReturnVal == NULL) {
        printf("--- Realloc couldn't find old ptr!  File %s Line %d\n", FromFile, FromLine);
    }
    
    pthread_mutex_unlock(&ClassMutex);
    
    return ReturnVal;
}

void MemAlloc::Free(void *ptr, char *FromFile, int FromLine) {
    
    bool Found = false;
    
    pthread_mutex_lock(&ClassMutex);
    
    for(int i = 0; i < MemAllocTableSize; i++) {
        if((Addr[i] == ptr) && (Size[i] > 0)) {
            if(  (((char *) Addr[i])[Size[i]] != 0xde) ||
                 (((char *) Addr[i])[Size[i] + 1] != 0xad) ||   
                 (((char *) Addr[i])[Size[i] + 2] != 0xbe) ||
                 (((char *) Addr[i])[Size[i] + 3] != 0xef) ) {
                printf("---Overwrote end of buffer!  Freed at %s, line %d\n", FromFile, FromLine);
                printf("   Alloced at %s, line %d\n", Filename[i].c_str(), Line[i]);
                printf("   Key = 0x%02x%02x%02x%02x\n", ((char *) Addr[i])[Size[i]],
                        ((char *) Addr[i])[Size[i] + 1], ((char *) Addr[i])[Size[i] + 2],
                        ((char *) Addr[i])[Size[i] + 3]);
            }
            Size[i] = 0;
            Filename[i] = string(FromFile);
            Line[i] = FromLine;
            free(ptr);
            Found = true;
        }
    }
    
    pthread_mutex_unlock(&ClassMutex);
    
    if(Found == false) {
        printf("---Freeing invalid ptr! File %s Line %d\n", FromFile, FromLine);
    }
}

#ifdef ALLOC_DEBUG
void *operator new(size_t size) {
    return MemAlloc::GetInstance()->Malloc(size, __FILE__, __LINE__);
}

void *operator new[](size_t size) {
    return MemAlloc::GetInstance()->Malloc(size, __FILE__, __LINE__);
}

void *operator new(size_t size, char *file, int line) {
    return MemAlloc::GetInstance()->Malloc(size, file, line);
}

void *operator new[](size_t size, char *file, int line) {
    return MemAlloc::GetInstance()->Malloc(size, file, line);
}

void operator delete(void *ptr) {
    MemAlloc::GetInstance()->Free(ptr, __FILE__, __LINE__);
}

void operator delete[](void *ptr) {
    MemAlloc::GetInstance()->Free(ptr, __FILE__, __LINE__);
}
#endif
