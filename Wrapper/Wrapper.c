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
#include <sys/reboot.h>
#include <linux/reboot.h>
#include <sys/wait.h>

int main(void) {
    int ReturnVal;

    system("mount /proc");
        
    while(1) {
        ReturnVal = WEXITSTATUS(system("/bin/rioplay"));
        
        printf("ReturnVal = %d\n", ReturnVal);
        
        if(ReturnVal < 0) {
            printf("Wrapper: system call failed!\n");
            exit(1);
        }
        else if(ReturnVal == 1) {
            /* Restart the system */
            reboot(LINUX_REBOOT_CMD_RESTART);
        }
    }
}
