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
#include "MenuScreen.hh"
#include "MemAlloc.hh"

MenuScreen::MenuScreen(void) {
    Title[0] = '\0';
    Options = NULL;
    NumOptions = 0;
    CurrentlySelected = 0;
    SelectedPos = 1;
    MaxMenus = 4;
}

MenuScreen::~MenuScreen(void) {
    int i;
    
    for(i = 0; i < NumOptions; i++) {
        __free(Options[i]);
    }
    if(Options != NULL) {
        __free(Options);
    }
}

void MenuScreen::SetTitle(char *NewTitle) {
    strncpy(Title, NewTitle, 64);
    Title[63]='\0';
}

void MenuScreen::AddOption(const char *NewOption) {
    if((NumOptions % 5) == 0) {
        Options = (char **) __realloc(Options, sizeof(char *) * (NumOptions + 5));
    }
    
    Options[NumOptions] = (char *) __malloc(sizeof(char) * (strlen(NewOption) + 1));
    
    strcpy(Options[NumOptions], NewOption);
    
    if(NumOptions == 0) {
        CurrentlySelected = 1; /* To make sure that CurrentlySelected doesn't
                                  get left at 0 when options are available */
    }
    NumOptions++;
}

void MenuScreen::ClearOptions(void) {
    int i;

    for(i = 0; i < NumOptions; i++) {
        __free(Options[i]);
    }
    if(Options != NULL) {
        __free(Options);
        Options = NULL;
    }
    NumOptions = 0;
    CurrentlySelected = 0;
    SelectedPos = 1;
}    

int MenuScreen::GetSelection(void) {
    return CurrentlySelected;
}

void MenuScreen::SetSelection(int inSelected) {
    CurrentlySelected = inSelected;
}

void MenuScreen::Advance(void) {
    CurrentlySelected++;
    SelectedPos++;
    if(CurrentlySelected > NumOptions) {
        if(NumOptions == 0) {
            CurrentlySelected = 0;
            SelectedPos = 1;
        }
        else {
            CurrentlySelected = 1;
            SelectedPos = 1;
        }
    }
    if(SelectedPos > MaxMenus) {
        SelectedPos = MaxMenus;
    }
    if(SelectedPos > NumOptions) {
        SelectedPos = NumOptions;
    }
}

void MenuScreen::Reverse(void) {
    CurrentlySelected--;
    SelectedPos--;
    if(CurrentlySelected < 1) {
        CurrentlySelected = NumOptions;
        SelectedPos = MaxMenus;
        if(SelectedPos > NumOptions) {
            SelectedPos = NumOptions;
        }
    }
    if(SelectedPos < 1) {
        SelectedPos = 1;
    }
}

void MenuScreen::Update(VFDLib &Display) {
    int i;
    int vfd_width = VFDLib::vfd_width;
    int vfd_height = VFDLib::vfd_height;
    int vfd_default_font = VFDLib::vfd_default_font;

    /* We put the string height here since we do a lot of calculations
     * from this unchanging value, rather than compute it every time we
     * decide to use it.
     */
    int StringHeight = Display.getTextHeight(vfd_default_font);

    /* Empegs have smaller screen real estate, so fewer menus */
    if (vfd_height < 64)
	    MaxMenus = 3;

    /* Set clip area to the whole screen */
    Display.setClipArea(0, 0, vfd_width, vfd_height);
    
    /* Clear screen */
    Display.clear(VFDSHADE_BLACK);
    
    /* Draw title */
    Display.drawText(Title, (vfd_width - 
	    Display.getTextWidth(Title, vfd_default_font)) / 2,
            1, vfd_default_font, -1);
    
    /* Inverse menu title */
    Display.invertRectangleClipped(0, 0, vfd_width, StringHeight + 1);
    
    /* Draw Border */
    if ((NumOptions < MaxMenus) || 
	(vfd_height >= 64)) /* Border blocks big menus */  
    	    Display.drawLineHorizClipped(vfd_height-1, 0, vfd_width-1, 
	    VFDSHADE_BRIGHT);
    Display.drawLineVertClipped(0, 0, vfd_height-1, VFDSHADE_BRIGHT);
    Display.drawLineVertClipped(vfd_width-1, 0, vfd_height-1, VFDSHADE_BRIGHT);

    /* Change clipping area */
    if ((NumOptions < MaxMenus) || 
	(vfd_height >= 64)) /* Clipping blocks big menus */  
      Display.setClipArea(3, StringHeight + 3, vfd_width-3, vfd_height-3);
    else /* allow for a little more room on shorter displays */
      Display.setClipArea(3, StringHeight + 3, vfd_width-3, vfd_height);

    if(NumOptions > 0) {
        /* Draw menu items */
        for(i = (CurrentlySelected - (SelectedPos - 1));
                (i < (CurrentlySelected + (MaxMenus - (SelectedPos - 1)))) &&
                (i <= NumOptions); i++) {
            Display.drawText(Options[i - 1], 4, StringHeight + 4 + 
                    ((StringHeight + 2) *
                    (i - (CurrentlySelected - (SelectedPos - 1)))),
                    vfd_default_font, -1);
        }

        /* Invert the selected menu item */
        /* (there's some ugly math here but it works...) */
        Display.invertRectangleClipped(2, StringHeight + 3 +
		((StringHeight + 2) * (SelectedPos - 1)), vfd_width-3,
                (StringHeight * 2) + 4 +
                ((StringHeight + 2) * (SelectedPos - 1)));
    }
}
