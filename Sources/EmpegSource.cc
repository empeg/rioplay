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
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "Screen.hh"
#include "MenuScreen.hh"
#include "KeyCodes.h"
#include "Log.hh"
#include "Globals.hh"
#include "Mp3Decoder.hh"
#include "FlacDecoder.hh"
#include "VorbisDecoder.hh"
#include "CommandHandler.hh"
#include "MemAlloc.hh"
#include <iostream.h>
#include <stl.h>

#include "EmpegSource.hh"

#define DBFILE "/empeg/var/database"
#define PLFILE "/empeg/var/playlists"
#define TAGFILE "/empeg/var/tags"
	


extern int errno;

EmpegSource::EmpegSource(void) {
    Handler = new EmpegCommandHandler(this);
    GatherDBInfo();
}

EmpegSource::~EmpegSource(void) {
    delete Handler;
}


void EmpegSource::PrintVector(ostream& os, const vector<ListElem>& vectarray) {
    for(
        vector<ListElem>::const_iterator it = vectarray.begin();
        it != vectarray.end();
        ++it
    ) {
        const ListElem& elem = *it;
        elem.print(os);
    }
}

int EmpegSource::FIDtoIndex(const vector<ListElem>& vectarray, 
	const int FID, const bool isLongFid) {
    int myFid = FID;
    int index = 0;
    bool found = false;

    for(
        vector<ListElem>::const_iterator it = vectarray.begin();
        it != vectarray.end();
        ++it
    )
    {
        const ListElem& elem = *it;
	if (isLongFid) {
	    myFid = FID >> 4;
	}
        if (elem.db_id_ == myFid)
	{
	    found = true;
	    break;
	}
	index++;
    }
    if (found == false) {
    	index = -1;
    }
	
    return index;
}

int EmpegSource::LocateFID(int FID) {
	char s[64];
	struct stat stbuf;
	int location = -1; /* default to FID not found */
	int error = 0;

	/*   Is tune on drive 0 or 1? (if neither then skip) */
	sprintf(s,"/drive0/fids/%x",FID);
	error = stat(s, &stbuf);

	if ((error != -1) && (stbuf.st_size > 0))
	{	/* We found the fid at drive "0" */
		location = 0;
	}
	else /* still not found yet */	
	{
		sprintf(s,"/drive1/fids/%x", FID);
		error = stat(s, &stbuf);

		if ((error != -1) && (stbuf.st_size > 0))
		{
			location = 1; /* We found the fid at drive "1" */
	    	}
	}

	return location;	
}

int EmpegSource::GetPListContents(int index, bool recurse, bool tunesonly,
        list<StringID>& plist)
{
	int offset = PLists[index].offset_;
	int length = PLists[index].length_;
	int z = 0, u = 0, location = 0;
	FILE *fp2 = NULL;
						
	fp2=fopen(PLFILE, "rb");
	if (fp2 == NULL) { return -1; }
	if (fseek(fp2, offset, SEEK_SET) != 0) { return -2; }	

	  for (u=0; u < (length/4); u++)
	  {
		fread(&z,4,1,fp2);

		if ((location = FIDisPList(z, true)) > -1)
		{	    
		    /* Got a playlist */
		    
		    if (!tunesonly) {
        		    plist.push_back(StringID(PLists[location].title_,
        		        PLists[location].index_));
                    }			
		    if (recurse)
		    {
		    	long position = ftell(fp2);
		        fclose(fp2); /* So we don't have tons of open pointers */
			fp2 = NULL;
			
		    	GetPListContents(location, recurse, tunesonly, plist);
			
			fp2=fopen(PLFILE, "rb");
			if (fp2 == NULL) { return -1; }
			if (fseek(fp2, position, SEEK_SET) != 0 ) { return -2; }
			
		    }
		}
		else if (tunesonly)
		{
		    /* Want a tune */
		    if ((location = FIDtoIndex(PlayerDB, z, true)) > -1) {
        		    plist.push_back(StringID(PlayerDB[location].title_, 
        		    	PlayerDB[location].index_));
                    }
		}
	  }
	fclose(fp2);
	return 0;
}

void EmpegSource::GatherDBInfo(void) {
        unsigned char   c;
        char    s[2048];
        char    *tags[50];
        int     i = 0,end_entry = 0,db_index = 0, plists = 0, ntracks = 0;
	long int offset = 0;
        FILE    *fp = NULL;

	ListElem *elem = NULL;

	printf("Gathering Empeg Database Info ...\n");

/*	Open tags file
 *	Read all tags from tags file
 *	Close tags file	
 *	Open database file
 *	For every entry in the database file
 *	   Collect tag information for entry
 *	   If the entry is a tune
 *	     Is tune on drive 0 or 1? (if neither then skip)
 *		Put id3 and file path in searchable db
 *	Close database file
 */
    
	/* Let's only have to do this once per startup, takes a while. */

	/* Open tags, read tags, close tags */

	printf("\tRetrieving Tag Database ...\n");
	
        fp=fopen(TAGFILE, "rb");
	if(fp == NULL) {
        	Log::GetInstance()->Post(LOG_FATAL, __FILE__, __LINE__,
                "Open TagsDB failed: %s", strerror(errno));
        	return;
	}

        i=0;
        while (fgets(s,sizeof(s), fp) != NULL) {
                s[strlen(s)-1]='\0';
                tags[i++]=(char *)strdup(s);
        }
        fclose(fp);


	/* Open database file */

	printf("\tRetrieving Song Database ...\n");

        fp=fopen(DBFILE, "rb");
	if(fp == NULL) {
        	Log::GetInstance()->Post(LOG_FATAL, __FILE__, __LINE__,
                "Open PlayerDB failed: %s", strerror(errno));
        	return;
	}

	elem = new ListElem();
        while ( fread(&c,1,1,fp) > 0 ) {
                if ( c == 0xFF ) {
                        end_entry=1;
                        db_index++;
                } else {
                        if (end_entry) {
				/* If the entry is a tune */
				if ((strcmp(elem->type_, "tune") == 0) &&
					(strlen(elem->artist_) > 0) &&
					(strlen(elem->title_) > 0))
				{
				   int location = -1;

				   /* "<< 4 | 0" gets music, "<< 4 | 1" gets info */
                                   i=(db_index-1) << 4 | 0;

				   location = LocateFID(i);
				   if (location == 0)
					sprintf(s,"/drive0/fids/%x",i);
				   else if (location == 1)
					sprintf(s,"/drive1/fids/%x",i);

				   if (location != -1)
				   {
 				 	/*   Put id3 and file path in searchable db */
					elem->index_ = ntracks;
					elem->db_id_ = (db_index-1);
    					elem->file_path_ = strnew(s);
					PlayerDB.push_back(*elem);
					ntracks++;
				   }
				}
				else if ((strcmp(elem->type_, "playlist") == 0) &&
					(strlen(elem->title_) > 0) &&
					(elem->length_ > 0))
				{
					elem->index_ = plists;
					elem->db_id_ = (db_index-1);

					/* Offset to playlist contents in "playlists" */
					elem->offset_ = offset;
					offset += elem->length_;
					
					PLists.push_back(*elem);
					plists++;
				}
				if (elem) { delete elem; }
				elem = new ListElem();
                                end_entry=0;
                        }
			
			/* Collect tag information for entry */
                        i=c;
                        fread(&c,1,1,fp);
                        fread(s,1,c,fp);
                        s[c]='\0';
			if (strcmp(tags[i], "type") == 0) {
                                elem->type_ = strnew(s);
			}
			else if (strcmp(tags[i], "artist") == 0) {
                                elem->artist_ = strnew(s);
			}
			else if (strcmp(tags[i], "source") == 0)
			{
                                if (strlen(s) == 0) {
                                        strcpy(s, "No Album");
                                }
                                elem->album_ = strnew(s);
			}	
			else if (strcmp(tags[i], "title") == 0) {
                                elem->title_ = strnew(s);
			}
			else if (strcmp(tags[i], "genre") == 0) {
				if (strlen(s) == 0) {
                                        strcpy(s, "No Genre");
                                }
				elem->genre_ = strnew(s);
			}	
			else if (strcmp(tags[i], "year") == 0) {
                                elem->year_ = strnew(s);
			}
			else if (strcmp(tags[i], "tracknr") == 0) {
                                elem->tracknum_ = strnew(s);
			}
			else if (strcmp(tags[i], "codec") == 0) {
                                elem->codec_ = strnew(s);
			}
			else if (strcmp(tags[i], "length") == 0) {
                                elem->length_ = atol(s);
			}
			else if (strcmp(tags[i], "duration") == 0) {
                                elem->duration_ = atol(s);
                        }
                }
        }
        fclose(fp);

	printf("\tLoaded %d Tracks and %d Playlists ...\n", 
		ntracks+1, plists+1);
				  

/*	If you want to print out the whole database (yikes!!!) */
//	print_vector(cerr, PlayerDB);
//	cerr << endl;
	
}


void EmpegSource::DoQuery(char *Field, char *Query) {
    set<string> UniqSet;
    set<string>::iterator setItr;
    
    /* Clear the list */
    List.clear();
    strcpy(SearchField, Field);
    
    for(
        vector<ListElem>::const_iterator it = PlayerDB.begin();
        it != PlayerDB.end();
        ++it
    ) {
        const ListElem& elem = *it;
	if (strcmp(Field, "artist") == 0)
		UniqSet.insert(string(elem.artist_));
	else if (strcmp(Field, "source") == 0)
		UniqSet.insert(string(elem.album_));
	else if (strcmp(Field, "genre") == 0)
		UniqSet.insert(string(elem.genre_));
	else if (strcmp(Field, "title") == 0)
		UniqSet.insert(string(elem.title_));
    }
 
    for( setItr = UniqSet.begin(); setItr != UniqSet.end(); ++setItr )
    {
    	List.push_back(*setItr);
    }

    sort(List.begin(), List.end());

    /* Someday I should figure out how to make vector<string> List use unique()
    	Rather than hassle with building a whole set, just for that purpose alone. */    
}

void EmpegSource::DoResults(char *Field, const char *Query) {

    /* Clear the list */
    SongList.clear();
    
    for(
        vector<ListElem>::const_iterator it = PlayerDB.begin();
        it != PlayerDB.end();
        ++it
    ) {
        const ListElem& elem = *it;
	if (strcmp(Field, "artist") == 0)
	{
		if ((strcmp(Query, elem.artist_) == 0) ||
			(strlen(Query) == 0))
		   SongList.push_back(StringID(elem.title_, elem.index_));
	}
	else if (strcmp(Field, "source") == 0)
	{
		if ((strcmp(Query, elem.album_) == 0) ||
			(strlen(Query) == 0))
		   SongList.push_back(StringID(elem.title_, elem.index_));
	}
	else if (strcmp(Field, "genre") == 0)
	{
		if ((strcmp(Query, elem.genre_) == 0) ||
			(strlen(Query) == 0))
		   SongList.push_back(StringID(elem.title_, elem.index_));
	}
	else if (strcmp(Field, "title") == 0)
	{
		if ((strcmp(Query, elem.title_) == 0) ||
			(strlen(Query) == 0))
		   SongList.push_back(StringID(elem.title_, elem.index_));
	}
    }
 
    /* Sort the list so it matches what was displayed on screen */
    SongList.sort();
}

void EmpegSource::DoPlaylists(int root, bool tunesonly) {
         
    /* Ideal functionality for playlists (Idea stolen from commercial Empeg software)

            menu items for root level playlists
                menu item for play all (play recursive tracks)
                menu item for list tracks (no recursion)
                    menu item for play these (play all tracks here)
                    menu items for immediate tracks
                        play 1 track
                menu items for immediate child playlists
                    upon selection of one of these
                        repeat ...
    */

        // Find the Root Playlist
        int location = root;
        list<StringID>::iterator IDiter;
	
	#define PL_PLAYALL    "Play All (Recursive)"
	#define PL_TRACKS     "List Tracks"
	#define PL_PLAYHERE   "Play These"

        // Clear the list
        SongList.clear();
        List.clear();
			
        if (!tunesonly) { // we want playlists
                SongList.push_back(StringID(PL_PLAYALL, location));
                SongList.push_back(StringID(PL_TRACKS, location));

                // the false means don't recurse
	        GetPListContents(location, false, tunesonly, SongList);		
                for(IDiter = SongList.begin(); IDiter != SongList.end();
                        IDiter++) {
                        char *title = (char *)((*IDiter).Str.c_str());		
                        List.push_back(title);
         	}
        }
        else { // we just want tunes (Play These)
                 List.push_back(PL_PLAYHERE);

                 // the false means don't recurse
	         GetPListContents(location, false, tunesonly, SongList);	
                 for(IDiter = SongList.begin(); IDiter != SongList.end();
                        IDiter++) {
                        char *title = (char *)((*IDiter).Str.c_str());		
                        List.push_back(title);
         	}
	}
}

int EmpegSource::PlaylistSelect(int selection) {
        list<StringID>::iterator IDiter;
        char * selectedMenu = NULL;
	int action = 0;
	int location = 0;
	int z = 0;
	
	selection = selection - 1;
	selectedMenu = (char *)(List[selection].c_str());

	#define ACT_NONE 0
	#define ACT_PLAY 1
	#define ACT_MENU 2

        if (strcmp(selectedMenu, PL_PLAYALL) == 0) {
	        location = (SongList.front()).ID; /* The root plist */
        	SongList.clear();
		/* Get a recursive list of songs */
                GetPListContents(location, true, true,
		        SongList);
		
		action = ACT_PLAY;			
	}
	else if (strcmp(selectedMenu, PL_TRACKS) == 0) {
	        location = (SongList.front()).ID; /* The root plist */
	        DoPlaylists(location, true);
		
		action = ACT_MENU;
	}
	else if (strcmp(selectedMenu, PL_PLAYHERE) == 0) {
		action = ACT_PLAY;
	}
	else {        // It's either a playlist or a track
                /* If it was a track, then the first item
		     in List() will be PL_PLAY_HERE */
		if (strcmp(List[0].c_str(), PL_PLAYHERE) == 0) {
		        // it was a track!
			string title;
			z = 0;
                        for(IDiter = SongList.begin();
                            ((z <= (selection)) && (IDiter != SongList.end())); 
			    IDiter++) {
                            z++;
			    if (z == selection) {
			          title = (*IDiter).Str;
				  location = (*IDiter).ID;
				  break;
			    }
                        }
			
                	SongList.clear(); // dump list
			SongList.push_back(StringID(title, location)); //only one track
			
       			action = ACT_PLAY;
		}
		else {  // It was a playlist
			z = 0;
                        for(IDiter = SongList.begin();
                            ((z <= (selection+1)) && (IDiter != SongList.end())); 
			    IDiter++) {
                            z++; 

			    if (z == (selection+1)) {
			          location = (*IDiter).ID;
				  break;
			    }
                        }
			DoPlaylists(location, false);
			
			action = ACT_MENU;
		}	
	}
	return action;
}


Tag EmpegSource::GetTag(int ID) {
    Tag ReturnVal;

    /* Set to zeros to be safe */
    bzero(&ReturnVal, sizeof(ReturnVal));
    
    strcpy(ReturnVal.Title, PlayerDB[ID].title_);     
    strcpy(ReturnVal.Artist, PlayerDB[ID].artist_);     
    strcpy(ReturnVal.Album, PlayerDB[ID].album_);     
    strcpy(ReturnVal.Year, PlayerDB[ID].year_);     
    strcpy(ReturnVal.Genre, PlayerDB[ID].genre_);     
    strcpy(ReturnVal.Codec, PlayerDB[ID].codec_);     
                
    /* Return the array of results */
    return ReturnVal;
}

EmpegCommandHandler::EmpegCommandHandler(EmpegSource *inEmpeg) {
    Empeg = inEmpeg;
}

EmpegCommandHandler::~EmpegCommandHandler(void) {
}

void EmpegCommandHandler::Handle(const unsigned long &Keycode) {
    list<StringID>::iterator IDiter;
    int Selection;
    
    if((Keycode == PANEL_MENU) || (Keycode == REMOTE_MENU)) {
        Globals::Display->RemoveTopScreen(&Menu);
        Globals::Remote.RemoveHandler();
        CurrentMenu = MENU_NONE;
        return;
    }
    else if((Keycode == PANEL_WHEEL_CW) || (Keycode == REMOTE_DOWN) || (Keycode == REMOTE_DOWN_REPEAT)) {
        Menu.Advance();
        Globals::Display->Update(&Menu);
        return;
    }
    else if((Keycode == PANEL_WHEEL_CCW) || (Keycode == REMOTE_UP) || (Keycode == REMOTE_UP_REPEAT)) {
        Menu.Reverse();
        Globals::Display->Update(&Menu);
        return;
    }
   switch(CurrentMenu) {
        case MENU_NONE: /* No Empeg menu displayed yet */
            Menu.ClearOptions();
            Menu.SetTitle("Select Music");
            Menu.AddOption("Artist");
            Menu.AddOption("Album");
            Menu.AddOption("Genre");
            Menu.AddOption("Title");
            Menu.AddOption("Playlist");
            CurrentMenu = MENU_ROOT;
            Globals::Display->SetTopScreen(&Menu);
            Globals::Display->Update(&Menu);
            return;
            break;
            
        case MENU_ROOT:
            Selection = Menu.GetSelection();
            Menu.ClearOptions();
            CurrentMenu = MENU_SELECTFROMGROUP;
            Globals::Display->ShowHourglass();
            switch(Selection) {
                case 1:
                    Menu.SetTitle("Select Artist");
                    Menu.AddOption("Play All");
                    Empeg->DoQuery("artist", "");
                    break;
                case 2:
                    Menu.SetTitle("Select Album");
                    Menu.AddOption("Play All");
                    Empeg->DoQuery("source", "");
                    break;
                case 3:
                    Menu.SetTitle("Select Genre");
                    Menu.AddOption("Play All");
                    Empeg->DoQuery("genre", "");
                    break;
                case 4:
                    Menu.SetTitle("Select Title");
                    Menu.AddOption("Play All");
                    Empeg->DoQuery("title", "");
                    break;
                case 5:
                    Menu.SetTitle("Select Playlist");
                    Empeg->DoPlaylists(0, false); // Start with root plist
                    CurrentMenu = MENU_PLAYLIST;
                    break;
            }
            for(vector<string>::iterator iter = Empeg->List.begin();
                    iter != Empeg->List.end();
                    iter++) {
                Menu.AddOption((*iter).c_str());
            }
            Globals::Display->Update(&Menu);
            return;
            break;
            
        case MENU_SELECTFROMGROUP:
            Globals::Display->ShowHourglass();

            /* Clear the playlist if the user used the "Play" button
               (leave the playlist intact if "Enter" was used) */
            if((Keycode == REMOTE_PLAY) || (Keycode == PANEL_PLAY)) {
                Globals::Playlist.Clear();
            }
	    
            Selection = Menu.GetSelection();
            if(Selection > 1) {
                Empeg->DoResults(Empeg->SearchField, Empeg->List[Selection - 2].c_str());
            }
            else {
                Empeg->DoResults(Empeg->SearchField, "");
            }
            for(IDiter = Empeg->SongList.begin();
                    IDiter != Empeg->SongList.end(); IDiter++) {
                Globals::Playlist.Enqueue(Empeg, (*IDiter).ID, (*IDiter).Str);
            }
            CurrentMenu = MENU_NONE;
            Globals::Display->RemoveTopScreen(&Menu);
            Globals::Display->ShowHourglass();
            Globals::Remote.RemoveHandler();
            Globals::Playlist.Play();
            return;
            break;
            
        case MENU_PLAYLIST:
	    int action = 0; 
	    action = Empeg->PlaylistSelect(Menu.GetSelection());
	    
            switch (action) {
	            case ACT_MENU:
                            Menu.ClearOptions();
                            Globals::Display->ShowHourglass();
                            for(vector<string>::iterator iter = Empeg->List.begin();
                                    iter != Empeg->List.end();
                                    iter++) {
                                Menu.AddOption((*iter).c_str());
                            }
                            Globals::Display->Update(&Menu);
 		            break;
	            case ACT_PLAY: 
                            for(IDiter = Empeg->SongList.begin(); IDiter != Empeg->SongList.end();
                                    IDiter++) {
                                Globals::Playlist.Enqueue(Empeg, (*IDiter).ID, (*IDiter).Str);
                            }
                            CurrentMenu = MENU_NONE;
                            Globals::Display->RemoveTopScreen(&Menu);
                            Globals::Display->ShowHourglass();
                            Globals::Remote.RemoveHandler();
                            Globals::Playlist.Play();		    
			    break;
	            case ACT_NONE:
                            CurrentMenu = MENU_NONE;
                            Globals::Display->RemoveTopScreen(&Menu);
                            Globals::Remote.RemoveHandler();
			    break;
            }

            return;
            break;
    }
    return;
}

void EmpegSource::Play(unsigned int ID) {

    /* ServerConn doesn't do jack for us in the case of a real file,
    	but OpenFile() will populate the LocalFD variable for us. */
    ServerConn = OpenFile(PlayerDB[ID].file_path_);
        
    /* If there's already a decoder running, kill it */
    if(Dec) {
        delete Dec;
        Dec = NULL;
    }
    
    /* Get track info */
    Tag TrackTag;
    TrackTag = GetTag(ID);
    
    /* Set title on status screen */
    Log::GetInstance()->Post(LOG_INFO, __FILE__, __LINE__,
            "Playing Title: %s Artist: %s Album: %s",
            TrackTag.Title, TrackTag.Artist, TrackTag.Album);
    Globals::Status->SetAttribs(TrackTag);
    Globals::Display->Update(Globals::Status);
    
    /* Determine audio encoding type and create an instance of the 
       appropriate decoder */
    if(strcmp(TrackTag.Codec, "mp3") == 0) {
        Dec = new Mp3Decoder(LocalFD, this);
    }
    else if(strcmp(TrackTag.Codec, "flac") == 0) {
        Dec = new FlacDecoder(LocalFD, this);
    }
    else if(strcmp(TrackTag.Codec, "ogg") == 0) {
        Dec = new VorbisDecoder(LocalFD, this);
    }
    else if(strcmp(TrackTag.Codec, "wma") == 0) {
        Log::GetInstance()->Post(LOG_ERROR, __FILE__, __LINE__,
                "WMA format is not supported (and probably won't ever be)");
        return;
    }
    else {
        Log::GetInstance()->Post(LOG_ERROR, __FILE__, __LINE__,
                "%s: Unknown codec = %s", PlayerDB[ID].file_path_, TrackTag.Codec);
        return;
    }
    
    /* Start the decoder process */
    Dec->Start();
}

CommandHandler *EmpegSource::GetHandler(void) {
    return Handler;
}
