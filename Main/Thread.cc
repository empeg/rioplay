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
#include <pthread.h>
#include "Thread.hh"
#include "MemAlloc.hh"

void *ThreadJump(void *arg) {
    Thread *T;
    
    T = (Thread *) arg;
    
    return T->ThreadMain(NULL);
}

Thread::Thread(void) {
    pthread_mutex_init(&ClassMutex, NULL);
    pthread_cond_init(&ClassCondition, NULL);
}

Thread::~Thread(void) {
}

void Thread::Start(void) {
    /* Create thread */
    pthread_create(&ThreadHandle, NULL, ThreadJump, this);
}

pthread_t *Thread::GetHandle(void) {
    return &ThreadHandle;
}
