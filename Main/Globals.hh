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

#include "AudioOutputDevice.hh"
#include "DisplayThread.hh"
#include "RemoteThread.hh"
#include "WebThread.hh"
#include "StatusScreen.hh"
#include "Playlist.hh"
#include "RioServerSource.hh"
#include "ShoutcastSource.hh"

#define HWTYP_RIORCV	0
#define HWNM_RIORCV "Rio Receiver"

#define HWTYP_EMPEG 	1
#define HWNM_EMPEG "Empeg"

class Globals {
public:
    static AudioOutputDevice *AudioOut;
    static DisplayThread Display;
    static RemoteThread Remote;
    static WebThread Web;
    static StatusScreen Status;
    static PlaylistClass Playlist;
    static RioServerSource RioServer;
    static ShoutcastSource Shoutcast;
    static int hw_type;
};

