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
#include <unistd.h>
#include <string.h>
#include "Playlist.hh"

Playlist::Playlist(void) {
    NumEntries = 0;
    Position = 0;
}

Playlist::~Playlist(void) {
}

void Playlist::Advance(void) {
    Position++;

    if(Position > NumEntries) {
        Position = 1;
    }
}
    
void Playlist::Reverse(void) {
    Position--;
    if(Position < 1) {
        Position = NumEntries;
    }
}

int Playlist::GetPosition(void) {
    return Position;
}

void Playlist::SetMetadata(char *Metadata, int MetadataLength) {
    /* Default behaviour is to ignore metadata.  Subclasses may
       override this if they have a need. */
    return;
}
