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

#ifndef TAG_H
#define TAG_H

#define TAG_KEY_FILEID       0
#define TAG_KEY_TITLE        1
#define TAG_KEY_ARTIST       2
#define TAG_KEY_ALBUM        3
#define TAG_KEY_YEAR         4
#define TAG_KEY_COMMENT      5
#define TAG_KEY_FILESIZE     6
#define TAG_KEY_SOURCETYPE   7
#define TAG_KEY_FILEPATH     8
#define TAG_KEY_GENRE        9
#define TAG_KEY_BITRATE      10
#define TAG_KEY_PLAYLISTNAME 11
#define TAG_KEY_CODEC        12
#define TAG_KEY_OFFSET       13
#define TAG_KEY_DURATION     14
#define TAG_KEY_TRACKNUMBER  15

typedef struct TagStruct {
    char Title[128];
    char Artist[128];
    char Album[128];
    char Year[5];
    char Comment[256];
    char Filesize[4];
    char SourceType[16];
    char FilePath[256];
    char Genre[32];
    char Bitrate[4];
    char PlaylistName[32];
    char Codec[8];
    char Offset[4];
    char Duration[4];
    char TrackNumber[4];
} Tag;

#endif /* #ifndef TAG_H */
