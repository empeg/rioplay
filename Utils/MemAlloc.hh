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

#ifndef MEMALLOC_HH
#define MEMALLOC_HH

#include <stdlib.h>
#include <string>

//#define ALLOC_DEBUG

#ifdef ALLOC_DEBUG
#define __malloc(x) MemAlloc::GetInstance()->Malloc(x, __FILE__, __LINE__)
#define __realloc(x,y) MemAlloc::GetInstance()->Realloc(x, y, __FILE__, __LINE__)
#define __free(x) MemAlloc::GetInstance()->Free(x, __FILE__, __LINE__)
#else
#define __malloc(x) malloc(x);
#define __realloc(x,y) realloc(x,y)
#define __free(x) free(x)
#endif

#define MemAllocTableSize 10240

class MemAlloc {
public:
    MemAlloc(void);
    ~MemAlloc(void);
    static MemAlloc *GetInstance(void);
    void *Malloc(size_t size, char *FromFile, int FromLine);
    void *Realloc(void *ptr, size_t size, char *FromFile, int FromLine);
    void Free(void *ptr, char *FromFile, int FromLine);
    
private:
    static MemAlloc *Instance;
    void *Addr[MemAllocTableSize];
    size_t Size[MemAllocTableSize];
    string Filename[MemAllocTableSize];
    int Line[MemAllocTableSize];
    int Index;
};

#endif /* MEMALLOC_HH */
