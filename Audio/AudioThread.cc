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
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "AudioThread.hh"
#include "MadCallbacks.hh"
#include "Commands.h"
#include "Tag.h"
#include "Playlist.hh"
#include "Http.h"
#include "Player.h"
#include "DisplayThread.hh"
#include "RemoteThread.hh"

extern DisplayThread Display;
extern RemoteThread Remote;

AudioThread::AudioThread(void) {
    /* Initialize class variables */
    SongFP = NULL;
    AudioFD = -1;
    CurrentTime.seconds = 0;
    CurrentTime.fraction = 0;
    CommandRequested = COMMAND_STOP;
    CommandActual = COMMAND_STOP;
    MetadataFrequency = 0;
    FirstRun = 0;
    LocalBuffer = NULL;
    
    /* Initialize mutexes and condition variables */
    pthread_mutex_init(&ClassMutex, NULL);
    pthread_cond_init(&ClassCondition, NULL);
    
    /* Open the audio device for output */
    AudioFD = open("/dev/audio", O_WRONLY);
    if (AudioFD < 0) {
        printf("Audio: Cannot open audio device\n");
        return;
    } 

    /* Initialize decoder */    
    mad_decoder_init(&MadDecoder, NULL, InputCallbackJump,
            NULL, NULL, OutputCallbackJump, ErrorCallbackJump, NULL);
}    

AudioThread::~AudioThread(void) {
}

void *AudioThread::ThreadMain(void *arg) {
    char Filename[256];
    Tag TrackTag;
    Playlist *PList;
    
    while(1) {
        if(GetRequestedCommand() == COMMAND_STOP) {
            /* We're stopped */
            SetActualCommand(COMMAND_STOP);

            /* Wait for command to change */
            pthread_mutex_lock(&ClassMutex);
            pthread_cond_wait(&ClassCondition, &ClassMutex);
            pthread_mutex_unlock(&ClassMutex);
        }
                
        /* Perform the requested command */
        switch(GetRequestedCommand()) {
            case COMMAND_CHANGESONG:
                SetRequestedCommand(COMMAND_PLAY);
                /* Yes I know there's no break here, I want it to spill into
                   the COMMAND_PLAY routine */
            case COMMAND_PLAY:
                /* Set the actual command */
                SetActualCommand(COMMAND_PLAY);
                
                /* Initialize the status display */
                Display.SetBottomScreen(&Status);
    
                /* Get Filename */
                if((PList = Remote.GetPlaylist()) == NULL) {
                    printf("Audio: No playlist selected\n");
                    SetRequestedCommand(COMMAND_STOP);
                    break;
                }
                
                if(PList->GetFilename(Filename, PList->GetPosition()) == NULL) {
                    printf("Audio: Nothing in playlist\n");
                    SetRequestedCommand(COMMAND_STOP);
                    break;
                }
                
                /* Get track info */
                TrackTag = PList->GetTag(PList->GetPosition());

                //printf("Audio: Currently playing:\n");
                //printf("  Title:  %s\n", TrackTag.Title);
                //printf("  Artist: %s\n", TrackTag.Artist);
                //printf("  Album:  %s\n", TrackTag.Album);
                Status.SetAttribs(TrackTag);
                Display.Update(&Status);

                /* Open MP3 File */
                SongFP = OpenFile(Filename);

                /* Set time to 0 */
                CurrentTime.seconds = 0;
                CurrentTime.fraction = 0;
                
                /* Prepare input function */
                FirstRun = 0;
                if(LocalBuffer != NULL) {
                    free(LocalBuffer);
                    LocalBuffer = NULL;
                }
                
                mad_decoder_run(&MadDecoder, MAD_DECODER_MODE_SYNC);

                mad_decoder_finish(&MadDecoder);

                /* Close Audio file */
                fclose(SongFP);
                
                /* Advance playlist */
                if(GetRequestedCommand() == COMMAND_PLAY) {
                    /* Only advance if we're in play mode (i.e. don't
                       advance if COMMAND_CHANGESONG was issued, since
                       the playlist would have already been adjusted in
                       that case */
                    PList->Advance();
                }
                
                break;
                
            case COMMAND_STOP:
                /* Audio already stopped, wait for another command */
                break;
                
            default:
                printf("Audio: Unknown command received\n");
                break;
        }
    }
}

void AudioThread::SetRequestedCommand(int Command) {
    /* Set requested command */
    pthread_mutex_lock(&ClassMutex);
    CommandRequested = Command;
    pthread_mutex_unlock(&ClassMutex);
    
    /* Signal audio thread indicating requested command has changed */
    pthread_cond_signal(&ClassCondition);
}

int AudioThread::GetRequestedCommand(void) {
    int ReturnVal;
    
    pthread_mutex_lock(&ClassMutex);
    ReturnVal = CommandRequested;
    pthread_mutex_unlock(&ClassMutex);
    
    return ReturnVal;
}

int AudioThread::GetActualCommand(void) {
    int ReturnVal;
    
    pthread_mutex_lock(&ClassMutex);
    ReturnVal = CommandActual;
    pthread_mutex_unlock(&ClassMutex);
    
    return ReturnVal;
}

void AudioThread::SetActualCommand(int Command) {
    /* Set actual command */
    pthread_mutex_lock(&ClassMutex);
    CommandActual = Command;
    pthread_mutex_unlock(&ClassMutex);
}
    
FILE *AudioThread::OpenFile(char *Filename) {
    FILE *fp;
    char Host[64];
    char Path[128];
    unsigned short Port;
    char TempString[128];
    
    if(strstr(Filename, "http://") != NULL) {
        /* Found web file/stream */
        
        /* Parse URL */
        HttpParseUrl(Filename, Host, &Port, Path);

        /* Open a connection to the Rio HTTP Server */    
        fp = HttpConnect(Host, Port);
        
        /* Send HTTP request */
        sprintf(TempString,
                "GET /%s HTTP/1.0\r\nHost: %s\r\nUser-Agent: RioPlay/%s\r\nicy-metadata:1\r\n\r\n",
                Path, Host, PLAYER_VER);
        fwrite(TempString, 1, strlen(TempString), fp);
        
        /* Find end of HTTP Header */
        MetadataFrequency = HttpSkipHeader(fp);
    
        /* Socket fd is now waiting with MP3 data on it */
    }
    else {
        /* Regular file */
        fp = fopen(Filename, "r");
    }
    
    return fp;
}

enum mad_flow AudioThread::InputCallback(void *ptr, struct mad_stream *stream) {
    int LeftoverBytes;
    int RetVal = 0;
    int MetadataLength, BytesRead, Temp;
    Tag TrackTag;
    Playlist *PList;

    if(FirstRun == 0) {
        FirstRun = 1;
        LeftoverBytes = 0;
    }
    else {
        LeftoverBytes = stream->bufend - stream->next_frame;
    }

    /* Move undecoded bytes to beginning of buffer */
    if(LeftoverBytes) {
        memmove(Buffer, stream->next_frame, LeftoverBytes);
    }

    if(MetadataFrequency > 0) {
        while(BUFFER_SIZE - (LeftoverBytes + RetVal) > MetadataFrequency) {
            /* Shoutcast metadata is embedded in the stream */
            if(LocalBuffer == NULL) {
                LocalBuffer = (char *) malloc(MetadataFrequency + 4096);
            }
            
            /* Read in the number of bytes equal to the frequency of the
               metadata updates */
            for(BytesRead = 0; BytesRead < MetadataFrequency; ) {
                Temp = fread(LocalBuffer + BytesRead, 1,
                    MetadataFrequency - BytesRead, SongFP);
                if(Temp < 0) {
                    perror("Audio: read"); 
                    return MAD_FLOW_STOP;
                }
                BytesRead += Temp;
            }
            
            /* Copy good audio to audio buffer */
            memcpy(Buffer + LeftoverBytes + RetVal, LocalBuffer,
                    BytesRead);
            RetVal += BytesRead;

            /* Read the metadata length */
            while(fread(LocalBuffer, 1, 1, SongFP) != 1);
            MetadataLength = ((unsigned char) LocalBuffer[0]) * 16;

            /* Read the metadata */
            for(BytesRead = 0; BytesRead != MetadataLength; ) {
                Temp = fread(LocalBuffer + BytesRead, 1,
                    MetadataLength - BytesRead, SongFP);
                if(Temp < 0) {
                    perror("Audio: read"); 
                    return MAD_FLOW_STOP;
                }
                BytesRead += Temp;
            }
                    

            /* Print the metadata */
            if(MetadataLength) {
                //printf("Audio: Metadata: ");
                //for(int i = 0; i < MetadataLength; i++) {
                //    printf("%c", LocalBuffer[i]);
                //}
                //printf("\n");
                
                if((PList = Remote.GetPlaylist()) == NULL) {
                    return MAD_FLOW_STOP;
                }
                PList->SetMetadata(LocalBuffer, MetadataLength);
                TrackTag = PList->GetTag(PList->GetPosition());
                Status.SetAttribs(TrackTag);
                Display.Update(&Status);
            }
        }    
    }
    else {
        /* No metadata, read as normal */
        RetVal = fread(Buffer + LeftoverBytes, 1,
                BUFFER_SIZE - LeftoverBytes, SongFP);
        if(RetVal <= 0) {
            perror("Read");
            /* Finished reading file - clean up here? */
            return MAD_FLOW_STOP;
        }
    }

    /* Signal MAD that buffer is ready */
    mad_stream_buffer(stream, Buffer, RetVal + LeftoverBytes);

    /* Successful completion */
    return MAD_FLOW_CONTINUE;
}

enum mad_flow AudioThread::OutputCallback(void *ptr,
        struct mad_header const *header, struct mad_pcm *pcm) {
    signed int LeftSample, RightSample;
    static char DecodedSample[4608];
    static int SamplePos = 0;
    static int LastTime = 0;
    int i;
    enum mad_flow ReturnVal = MAD_FLOW_CONTINUE;

    for (i = 0; i < pcm->length; i++) {
        LeftSample = ScaleSample(pcm->samples[0][i]);
        RightSample = ScaleSample(pcm->samples[1][i]);

        DecodedSample[SamplePos++] = LeftSample & 0xff;
        DecodedSample[SamplePos++] = (LeftSample >> 8) & 0xff;
        DecodedSample[SamplePos++] = RightSample & 0xff;
        DecodedSample[SamplePos++] = (RightSample >> 8) & 0xff;

        if (SamplePos == 4608) {
            /* Buffer full, write it to the audio device */
            write(AudioFD, DecodedSample, 4608);

            if(GetRequestedCommand() == COMMAND_PAUSE) {
                SetActualCommand(COMMAND_PAUSE);
                pthread_mutex_lock(&ClassMutex);
                pthread_cond_wait(&ClassCondition, &ClassMutex);
                pthread_mutex_unlock(&ClassMutex);
                SetActualCommand(COMMAND_PLAY);
            }
            if(GetRequestedCommand() != COMMAND_PLAY) {
                ReturnVal = MAD_FLOW_STOP;
            }
            
            /* Lock */
            pthread_mutex_lock(&ClassMutex);
            
            /* Update time display */
            mad_timer_add(&CurrentTime, header->duration);
            if (LastTime != CurrentTime.seconds) {
                Status.SetTime(CurrentTime.seconds / 60,
                        CurrentTime.seconds % 60);
                LastTime = CurrentTime.seconds;

                /* Signal the display thread that the time has changed */
                Display.Update(&Status);
            }
                
            /* Unlock */
            pthread_mutex_unlock(&ClassMutex);
                
            /* Force a yield so display thread can run */
            sched_yield();

            /* Reset position in static sample buffer */
            SamplePos = 0;
        }
    }

    return ReturnVal;
}

enum mad_flow AudioThread::ErrorCallback(void *ptr, struct mad_stream *stream,
        struct mad_frame *frame) {
    printf("Audio: Decoding error %d - %s\n", stream->error, mad_stream_errorstr(stream));
    fflush(stdout);

    return MAD_FLOW_CONTINUE;
}

