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

#ifndef SWARMSCREEN_HH
#define SWARMSCREEN_HH

#include "Screen.hh"
#include "VFDLib.hh"

#define BEES	30	/* number of bees */
#define TIMES	3	/* number of time positions recorded */
#define BEEACC	3	/* acceleration of bees */
#define WASPACC 5	/* maximum acceleration of wasp */
#define BEEVEL	8	/* maximum bee velocity */
#define WASPVEL 9	/* maximum wasp velocity */
#define BORDER	5	/* wasp won't go closer than this to the edges */

/* Macros */
#define X(t,b)	(x[t*BEES + b])		/* addressing into dynamic array */
#define Y(t,b)	(y[t*BEES + b])		/* addressing into dynamic array */
#define RAND(v)	((random() % v) - (v/2)) /* random number around 0 */


class SwarmScreen : public Screen {
public:
    SwarmScreen(void);
    ~SwarmScreen(void);
    virtual void Update(VFDLib &Display);
   
private:
    void * Safe_Malloc(int bytes);
    int        b;		/* bee index */
    int        *x, *y;		/* bee positions x[time][bee#] */
    int        *xv, *yv;	/* bee velocities xv[bee#] */
    int        wx[3], wy[3];	/* wasp positions */
    int        wxv, wyv;	/* wasp velocity */
    bool       init;

};

#endif /* #ifndef SWARMSCREEN_HH */
