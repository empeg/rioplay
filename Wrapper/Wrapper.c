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
#include <sys/types.h>
#include <sys/mount.h>
#include <errno.h>

int main(void) {
    int ReturnVal;
    pid_t RioplayPID;
    int status;

    if(mount("none", "/proc", "proc", 0, NULL) != 0) {
        perror("mount");
    }
        
    while(1) {
        ReturnVal = 0;
        RioplayPID = fork();
        
        if(RioplayPID == 0) {
            /* This is the child process */
            execve("/bin/rioplay", NULL, NULL);
        }
        else {
            /* This is the init process */
            printf("init: rioplay started with PID %d\n", RioplayPID);
            
            waitpid(RioplayPID, &status, 0);
            
            if(WIFEXITED(status)) {
                ReturnVal = WEXITSTATUS(status);
            }

            printf("\ninit: rioplay exited with status %d\n", ReturnVal);

            if(ReturnVal < 0) {
                printf("init: error returned from waitpid()\n");
                exit(1);
            }
            else if(ReturnVal == 1) {
                /* Restart the system */
                printf("init: rebooting...\n\n");
                fflush(stdout);
                
                /* Give the system a chance to print out our messages */
                usleep(100000);
                
                /* Restart the system */
                reboot(LINUX_REBOOT_CMD_RESTART);
            }
        }
    }
}
