// Rioplay - Replacement Rio Receiver player software
// Copyright (C) 2002 David Flowerday
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// version 2 as published by the Free Software Foundation
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

#ifndef EMPEGSOURCE_H
#define EMPEGSOURCE_H

#include <vector>
#include <list>
#include <string>
#include "InputSource.hh"
#include "Tag.h"
#include "CommandHandler.hh"
#include "MenuScreen.hh"
#include "RioServerSource.hh"


class EmpegSource;

inline char* strnew(const char* str) {
    char* newstr = 0;
    if (str) {
        newstr = new char[strlen(str) + 1];
        strcpy(newstr, str);
    }
    return newstr;
}

struct ListElem {
    int index_;
    int db_id_;
    char* type_;
    char* file_path_;
    char* artist_;
    char* album_;
    char* title_;
    char* genre_;
    char* year_;
    char* tracknum_;
    char* codec_;
    long int length_;
    long int duration_;
    long int offset_;	/* Offset to playlist's contents */

    ListElem() :
        index_(0),
        db_id_(0),
        type_(strnew("")),
        file_path_(strnew("")),
	artist_(strnew("")),
	album_(strnew("No Album")),
	title_(strnew("")),
	genre_(strnew("No Genre")),
	year_(strnew("")),
	tracknum_(strnew("")),
	codec_(strnew("")),
	length_(0),
	duration_(0),
	offset_(0)
    { }
    ListElem(const ListElem& elem) :
        index_(elem.index_),
        db_id_(elem.db_id_),
        type_(strnew(elem.type_)),
        file_path_(strnew(elem.file_path_)),
        artist_(strnew(elem.artist_)),
        album_(strnew(elem.album_)),
        title_(strnew(elem.title_)),
        genre_(strnew(elem.genre_)),
        year_(strnew(elem.year_)),
        tracknum_(strnew(elem.tracknum_)),
        codec_(strnew(elem.codec_)),
        length_(elem.length_),
	duration_(elem.duration_),
	offset_(elem.offset_)
    { }
    ~ListElem() { 
        delete[] type_;
        delete[] file_path_;
        delete[] artist_;
        delete[] album_;
        delete[] title_;
        delete[] genre_;
        delete[] year_;
        delete[] tracknum_;
        delete[] codec_;
    }

    void print(ostream& os) const {
        os << "(" << index_ << " - " << artist_ << " - " << title_ << ")";
    }
};



class EmpegCommandHandler : public CommandHandler {
public:
    EmpegCommandHandler(EmpegSource *inEmpeg);
    ~EmpegCommandHandler(void);
    void Handle(const unsigned long &Keycode);
    int GetCurrentMenu() {return CurrentMenu; }
private:
    MenuScreen Menu;
    enum MenuTypes {
        MENU_NONE = 0,
        MENU_ROOT,
        MENU_SELECTFROMGROUP,
        MENU_PLAYLIST
    };
        
    int CurrentMenu;
    EmpegSource *Empeg;
};

class EmpegSource : public InputSource {
friend EmpegCommandHandler;
public:
    EmpegSource(void);
    ~EmpegSource(void);
    void Play(unsigned int ID);
    Tag GetTag(int ID);
    void DoQuery(char *Field, char *Query);
    CommandHandler *GetHandler(void);
    void GatherDBInfo(void);

private:    
    void DoResults(char *Field, const char *Query);
    void DoPlaylists(int ID, bool tunesonly);
    int PlaylistSelect(int ID);
    int LocateFID(int FID);
    void PrintVector(ostream& os, const vector<ListElem>& vectarray);
    int FIDtoIndex(const vector<ListElem>& vectarray, const int FID, const bool isLongFid);
    int FIDisPList(const int FID, const bool isLongFid) { return (FIDtoIndex(PLists, FID, isLongFid)); }
    int GetPListContents(int index, bool recurse, bool tunesonly, list<StringID>& plist);
    char SearchField[8];
    list<StringID> SongList;
    vector<string> List;
    vector<ListElem> PlayerDB;
    vector<ListElem> PLists;
    EmpegCommandHandler *Handler;
};    

#endif /* #ifndef EMPEGSOURCE_H */
