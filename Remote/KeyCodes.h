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

#ifndef KEYCODES_H
#define KEYCODES_H

/* Panel codes (Buttons on the box itself) */
#define PANEL_POWER_DOWN 0x00010018
#define PANEL_POWER_UP   0x00010019
#define PANEL_PLAY       0x00010004
#define PANEL_STOP       0x00010002
#define PANEL_FORWARD    0x00010008
#define PANEL_REVERSE    0x0001000a
#define PANEL_VOL_UP     0x00028000
#define PANEL_VOL_DOWN   0x00028001
#define PANEL_MENU       0x00010012
#define PANEL_WHEEL_CW   0x00028000
#define PANEL_WHEEL_CCW  0x00028001
#define PANEL_WHEEL_BUTTON 0x00010010

/* Remote codes */
#define REMOTE_POWER     0x00000184
#define REMOTE_PLAY      0x00000185
#define REMOTE_STOP      0x00000190
#define REMOTE_FORWARD   0x00000187
#define REMOTE_REVERSE   0x00000183
#define REMOTE_VOL_UP    0x0000018a
#define REMOTE_VOL_DOWN  0x00000181
#define REMOTE_VOL_UP_REPEAT    0x8000018a
#define REMOTE_VOL_DOWN_REPEAT  0x80000181
#define REMOTE_MENU      0x00000188
#define REMOTE_UP        0x0000018c
#define REMOTE_DOWN      0x00000186
#define REMOTE_UP_REPEAT        0x8000018c
#define REMOTE_DOWN_REPEAT      0x80000186
#define REMOTE_ENTER     0x00000182

#endif /* #ifndef KEYCODES_H */
