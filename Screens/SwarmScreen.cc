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
#include <sys/time.h>
#include "SwarmScreen.hh"
#include "MemAlloc.hh"


SwarmScreen::SwarmScreen(void) {
    int vfd_width = VFDLib::vfd_width;
    int vfd_height = VFDLib::vfd_height;

    init = true;        

    /* Allocate memory. */
    x = (int *) Safe_Malloc(sizeof(int) * BEES * TIMES);
    y = (int *) Safe_Malloc(sizeof(int) * BEES * TIMES);
    xv = (int *) Safe_Malloc(sizeof(int) * BEES);
    yv = (int *) Safe_Malloc(sizeof(int) * BEES);

 
    /* Initialize point positions, velocities, etc. */

    /* wasp */
    wx[0] = BORDER + random() % (vfd_width - 2*BORDER);
    wy[0] = BORDER + random() % (vfd_height - 2*BORDER);

    wx[1] = wx[0];
    wy[1] = wy[0];
    wxv = 0;
    wyv = 0;

    /* bees */
    for (b = 0 ; b < BEES ; b++)
    {
        X(0,b) = random() % vfd_width;
        X(1,b) = X(0,b);
        Y(0,b) = random() % vfd_height;
        Y(1,b) = Y(0,b);
        xv[b] = RAND(7);
        yv[b] = RAND(7);
    }
}

SwarmScreen::~SwarmScreen(void) {
    /* Free Memory */
    free(x);
    free(y);
    free(xv);
    free(yv);
}


void SwarmScreen::Update(VFDLib &Display) {
    int vfd_width = VFDLib::vfd_width;
    int vfd_height = VFDLib::vfd_height;
    int	dx,dy,distance;
    int bx1, by1, bx2, by2, ex1, ey1, ex2, ey2;  
    static struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 40000;


    /* <=- Wasp -=> */
    /* Age the position arrays. */
    wx[2] = wx[1];
    wx[1] = wx[0];
    wy[2] = wy[1];
    wy[1] = wy[0];

    /* Accelerate */
    wxv += RAND(WASPACC);
    wyv += RAND(WASPACC);

    /* Speed Limit Checks */
    if (wxv > WASPVEL) {
        wxv = WASPVEL;
    }
    if (wxv < -WASPVEL) {
        wxv = -WASPVEL;
    }
    if (wyv > WASPVEL) {
        wyv = WASPVEL;
    }
    if (wyv < -WASPVEL) {
        wyv = -WASPVEL;
    }
    
    /* Move */
    wx[0] = wx[1] + wxv;
    wy[0] = wy[1] + wyv;

    /* Bounce Checks */
    if ((wx[0] < BORDER) || (wx[0] > vfd_width-BORDER-1))
    {
	wxv = -wxv;
	wx[0] += wxv<<1;
    }
    if ((wy[0] < BORDER) || (wy[0] > vfd_height-BORDER-1))
    {
	wyv = -wyv;
	wy[0] += wyv<<1;
    }

    /* Don't let things settle down. */
    if (BEES > 0) /* Avoid division by 0. */
    {
        xv[random() % BEES] += RAND(3);
        yv[random() % BEES] += RAND(3);
    }

    /* <=- Bees -=> */
    for (b = 0 ; b < BEES ; b++)
    {
        /* Age the arrays. */
        X(2,b) = X(1,b);
        X(1,b) = X(0,b);
        Y(2,b) = Y(1,b);
        Y(1,b) = Y(0,b);

        /* Accelerate */
        dx = wx[1] - X(1,b);
        dy = wy[1] - Y(1,b);
        distance = abs(dx)+abs(dy); /* approximation */
        if (distance == 0) {
            distance = 1;
        }
        xv[b] += (dx*BEEACC)/distance;
        yv[b] += (dy*BEEACC)/distance;

        /* Speed Limit Checks */
        if (xv[b] > BEEVEL) {
            xv[b] = BEEVEL;
        }
        if (xv[b] < -BEEVEL) {
            xv[b] = -BEEVEL;
        }
        if (yv[b] > BEEVEL) {
            yv[b] = BEEVEL;
        }
        if (yv[b] < -BEEVEL) {
            yv[b] = -BEEVEL;
        }
	
        /* Move */
        X(0,b) = X(1,b) + xv[b];
        Y(0,b) = Y(1,b) + yv[b];

   	/* Bees */

        /* Fill the segment lists. */
        bx1 = X(0,b);
        by1 = Y(0,b);
        bx2 = X(1,b);
        by2 = Y(1,b);
        ex1 = X(1,b);
        ey1 = Y(1,b);
        ex2 = X(2,b);
        ey2 = Y(2,b);

        if (init == false) {
             Display.drawLineClipped(ex1, ey1, ex2, ey2, VFDSHADE_BLACK);
        }
        else {
             init = false;
        }
        Display.drawLineClipped(bx1, by1, bx2, by2, VFDSHADE_MEDIUM);
    }
	
    /* Erase previous, draw current, sync for smoothness. */


    /* Wasp */
    if (init == false) {
         Display.drawLineClipped(wx[1], wy[1], wx[2], wy[2], VFDSHADE_BLACK);
    }
    Display.drawLineClipped(wx[0], wy[0], wx[1], wy[1], VFDSHADE_BRIGHT);
    
    select(0, 0, 0, 0, &tv); // Take a little nap ... 

}


/* Check the return code of malloc(). */
void * SwarmScreen::Safe_Malloc(int bytes)
{
    void *pointer;

    pointer = (void *) malloc(bytes);
    if (NULL == pointer)
    {
	fprintf(stderr, "Error allocating %d bytes.\n", bytes);
	exit(-1);
    }

    return(pointer);
}



