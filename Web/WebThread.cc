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
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include "WebThread.hh"
#include "Log.hh"
#include "MemAlloc.hh"

extern int errno;

WebThread::WebThread(void) {
}

WebThread::~WebThread(void) {
}

void *WebThread::ThreadMain(void *arg) {
    int Sock, newSock;
    unsigned int cliLen;
    struct sockaddr_in cliAddr, servAddr;
    char TempString[1024];
    Log *LogI = Log::GetInstance();
    int One = 1;
    FILE *fp;
    FILE *loadavg;

    /* Create socket */
    Sock = socket(AF_INET, SOCK_STREAM, 0);
    if(Sock < 0) {
        LogI->Post(LOG_ERROR, __FILE__, __LINE__,
                "socket() failed: %s", strerror(errno));
        pthread_exit(0);
    }
    
    setsockopt(Sock, SOL_SOCKET, SO_REUSEADDR, &One, sizeof(One));
  
    /* Bind server port */
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(80);
    
    if(bind(Sock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0) {
        LogI->Post(LOG_ERROR, __FILE__, __LINE__,
                "bind() failed: %s", strerror(errno));
        pthread_exit(0);
    }

    if(listen(Sock, 5) < 0) {
        LogI->Post(LOG_ERROR, __FILE__, __LINE__,
                "listen() failed: %s", strerror(errno));
        pthread_exit(0);
    }
  
    while(1) {
        cliLen = sizeof(cliAddr);
        newSock = accept(Sock, (struct sockaddr *) &cliAddr, &cliLen);
        if(newSock < 0) {
            LogI->Post(LOG_ERROR, __FILE__, __LINE__,
                    "accept() failed: %s", strerror(errno));
            pthread_exit(0);
        }

        fp = fdopen(newSock, "r+");
        
        while(fgets(TempString, 1024, fp)[0] != '\r');
        
        fprintf(fp, "HTTP/1.1 200 OK\r\n\r\n");
        
        fprintf(fp, "<http><head><title>RioPlay Event Log</title></head><font size=+2>RioPlay Event Log</font><br><br>\n");
        
        time_t gmttime = time(NULL);
        fprintf(fp, "RioPlay time: %s<br><br>\n", asctime(localtime(&gmttime)));
        
        loadavg = fopen("/proc/loadavg", "r");
        if(loadavg != NULL) {
            fgets(TempString, 1024, loadavg);
            fclose(loadavg);
            fprintf(fp, "Load Average: %s<br><br>\n", TempString);
        }
        loadavg = fopen("/proc/meminfo", "r");
        if(loadavg != NULL) {
            fprintf(fp, "Memory info:<br>\n<pre>");
            while(fgets(TempString, 1024, loadavg)) {
                fprintf(fp, "%s", TempString);
            }
            fclose(loadavg);
            fprintf(fp, "</pre><br><br>\n");
        }
        
        fprintf(fp, "<table border=1 cellpadding=2><tr><td>Severity</td><td>File</td><td>Line</td><td>Message</td></tr>\n");
        
        for(int i = 1; i <= LogI->GetNumEvents(); i++) {
            switch(LogI->GetSeverity(i)) {
                case LOG_INFO:
                    fprintf(fp, "<tr><td bgcolor=green>Info</td>");
                    break;
                    
                case LOG_WARNING:
                    fprintf(fp, "<tr><td bgcolor=yellow>Warning</td>");
                    break;
                    
                case LOG_ERROR:
                    fprintf(fp, "<tr><td bgcolor=orange>Error</td>");
                    break;
                    
                case LOG_FATAL:
                default:
                    fprintf(fp, "<tr><td bgcolor=red>Fatal</td>");
                    break;
            }
                    
            fprintf(fp, "<td>%s</td><td>%d</td><td>%s</td></tr>\n",
                    LogI->GetFile(i), LogI->GetLine(i), LogI->GetMessage(i));
        }
        
        fprintf(fp, "</table></html>\n");
        
        fclose(fp);
    }
}
