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

void MenuScreen::AddOption(char *NewOption) {
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
    if(SelectedPos > 4) {
        SelectedPos = 4;
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
        SelectedPos = 4;
        if(SelectedPos > NumOptions) {
            SelectedPos = NumOptions;
        }
    }
    if(SelectedPos < 1) {
        SelectedPos = 1;
    }
}

void MenuScreen::Update(char *Display) {
    int i;

    /* Clear screen */
    bzero(Display, 4096);
    
    /* Draw title */
    MenuFont.DrawStringCentered(Display, Title, 1);
    
    /* Inverse menu title */
    Inverse(Display, 0, 0, 127, MenuFont.GetHeight() + 1);
    
    /* Draw Border */
    DrawHorizontalLine(Display, 0, 63, 128);
    DrawVerticalLine(Display, 0, 0, 64);
    DrawVerticalLine(Display, 127, 0, 64);

    /* Draw menu items */
/*    for(i = CurrentlySelected; (i < (CurrentlySelected + 4)) && (i <= NumOptions); i++) {
        MenuFont.DrawString(Display, Options[i - 1], 4,
                MenuFont.GetHeight() + 4 + ((MenuFont.GetHeight() + 2) * (i - CurrentlySelected)));
    }*/
    for(i = (CurrentlySelected - (SelectedPos - 1)); (i < (CurrentlySelected + (4 - (SelectedPos - 1)))) && (i <= NumOptions); i++) {
        MenuFont.DrawString(Display, Options[i - 1], 4,
                MenuFont.GetHeight() + 4 + ((MenuFont.GetHeight() + 2) * (i - (CurrentlySelected - (SelectedPos - 1)))));
    }
    
    /* Invert the selected menu item */
    /* (there's some ugly math here but it works...) */
    Inverse(Display, 2, MenuFont.GetHeight() + 3 +
            ((MenuFont.GetHeight() + 2) * (SelectedPos - 1)), 125,
            (MenuFont.GetHeight() * 2) + 3 + ((MenuFont.GetHeight() + 2) *
            (SelectedPos - 1)));
}
