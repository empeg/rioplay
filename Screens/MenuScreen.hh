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

#ifndef MENUSCREEN_HH
#define MENUSCREEN_HH

#include "Screen.hh"
#include "VFDLib.hh"

class MenuScreen : public Screen {
public:
    MenuScreen(void);
    ~MenuScreen(void);
    virtual void Update(VFDLib &Display);
    void SetTitle(char *NewTitle);
    void AddOption(const char *NewOption);
    void ClearOptions(void);
    int GetSelection(void);
    void SetSelection(int inSelected);
    void Advance(void);
    void Reverse(void);

private:
    char Title[64];
    char **Options;
    int NumOptions;
    int CurrentlySelected;
    int SelectedPos;
    int MaxMenus;
};

#endif /* #ifndef MENUSCREEN_HH */
