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

#ifndef WEBTHREAD_HH
#define WEBTHREAD_HH

#include "Thread.hh"

class WebThread : public Thread {
public:
    WebThread(void);
    ~WebThread(void);
    virtual void *ThreadMain(void *arg);
    
private:
};

#endif /* #ifndef WEBTHREAD_HH */
