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
#include <errno.h>
#include "AudioThread.hh"
#include "MadCallbacks.hh"
#include "Commands.h"
#include "Tag.h"
#include "Playlist.hh"
#include "Http.hh"
#include "Player.h"
#include "DisplayThread.hh"
#include "RemoteThread.hh"
#include "Log.hh"
#include "StatusScreen.hh"
#include "BufferClass.hh"
#include "MemAlloc.hh"

extern int errno;

extern DisplayThread Display;
extern RemoteThread Remote;

AudioThread::AudioThread(void) {
    /* Initialize class variables */
    SongFD = -1;
    AudioFD = -1;
    CurrentTime.seconds = 0;
    CurrentTime.fraction = 0;
    CommandRequested = COMMAND_STOP;
    CommandActual = COMMAND_STOP;
    MetadataFrequency = 0;
    FirstRun = 0;
    LocalBuffer = NULL;
    BufferSize = BUFFER_SIZE;
    ExtBuffer = NULL;
    Mp3SampleRate = 44100;
    
    Buffer = (unsigned char *) __malloc(BufferSize);
    
    /* Initialize mutexes and condition variables */
    pthread_mutex_init(&ClassMutex, NULL);
    pthread_cond_init(&ClassCondition, NULL);
    
    /* Open the audio device for output */
    AudioFD = open("/dev/audio", O_WRONLY);
    if (AudioFD < 0) {
        Log::GetInstance()->Post(LOG_FATAL, __FILE__, __LINE__,
                "Cannot open audio device");
        return;
    } 

    /* Initialize decoder */    
    mad_decoder_init(&MadDecoder, NULL, InputCallbackJump,
            NULL, NULL, OutputCallbackJump, ErrorCallbackJump, NULL);
}    

AudioThread::~AudioThread(void) {
    __free(Buffer);
    
    if(LocalBuffer != NULL) {
        __free(LocalBuffer);
    }
}

void *AudioThread::ThreadMain(void *arg) {
    char Filename[256];
    Tag TrackTag;
    Playlist *PList;
    HttpConnection *Http;
    
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
                    Log::GetInstance()->Post(LOG_WARNING, __FILE__, __LINE__,
                            "Audio: No playlist selected");
                    SetRequestedCommand(COMMAND_STOP);
                    break;
                }
                
                if(PList->GetFilename(Filename, PList->GetPosition()) == NULL) {
                    Log::GetInstance()->Post(LOG_WARNING, __FILE__, __LINE__,
                            "Audio: Nothing in playlist");
                    SetRequestedCommand(COMMAND_STOP);
                    break;
                }
                
                /* Get track info */
                TrackTag = PList->GetTag(PList->GetPosition());

                Log::GetInstance()->Post(LOG_INFO, __FILE__, __LINE__,
                        "Playing Title: %s Artist: %s Album: %s",
                        TrackTag.Title, TrackTag.Artist, TrackTag.Album);
                Status.SetAttribs(TrackTag);
                Display.Update(&Status);

                /* Open MP3 File */
                Http = OpenFile(Filename);
                if((SongFD = Http->GetDescriptor()) < 0) {
                    /* Invalid file */
                    Log::GetInstance()->Post(LOG_ERROR, __FILE__, __LINE__,
                            "Could not open file %s", Filename);
                    
                    SetRequestedCommand(COMMAND_STOP);
                    break;
                }

                /* Set up external buffer thread */
                ExtBuffer = new BufferClass(SongFD, 81920);
                if(MetadataFrequency > 0) {
                    /* Fill the buffer before we start playback */
                    ExtBuffer->Prebuffer();
                }
                
                /* Set time to 0 */
                CurrentTime.seconds = 0;
                CurrentTime.fraction = 0;
                
                /* Prepare input function */
                FirstRun = 0;
                if(LocalBuffer != NULL) {
                    __free(LocalBuffer);
                    LocalBuffer = NULL;
                }
                
                mad_decoder_run(&MadDecoder, MAD_DECODER_MODE_SYNC);

                mad_decoder_finish(&MadDecoder);

                Log::GetInstance()->Post(LOG_INFO, __FILE__, __LINE__,
                        "Decoder finished");
                
                /* Close Audio file */
                delete ExtBuffer;
                delete Http;
                
                /* Advance playlist */
                if(GetRequestedCommand() == COMMAND_PLAY) {
                    /* Only advance if we're in play mode (i.e. don't
                       advance if COMMAND_CHANGESONG was issued, since
                       the playlist would have already been adjusted in
                       that case */
                    PList->Advance();
                }
                else if(GetRequestedCommand() == COMMAND_STOP) {
                    Status.SetTime(0, 0);
                    Display.Update(&Status);
                }
                
                break;
                
            case COMMAND_STOP:
                /* Audio already stopped, wait for another command */
                break;
                
            default:
                Log::GetInstance()->Post(LOG_WARNING, __FILE__, __LINE__,
                    "Unknown command received");
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
    
HttpConnection *AudioThread::OpenFile(char *Filename) {
    int fd;
    HttpConnection *Http = NULL;
    
    if(strstr(Filename, "http://") != NULL) {
        /* Found web file/stream */
        
        Http = new HttpConnection(Filename);
        
        /* Open a connection to the Rio HTTP Server */    
        fd = Http->Connect();
        if(fd < 0) {
            Log::GetInstance()->Post(LOG_FATAL, __FILE__, __LINE__,
                    "Couldn't connect to URL %s", Filename);
            return NULL;
        }
        
        /* Find end of HTTP Header */
        MetadataFrequency = Http->SkipHeader();
    
        /* Socket fd is now waiting with MP3 data on it */
    }
    else {
        /* Regular file */
        fd = open(Filename, O_RDONLY);
        if(fd < 0) {
            Log::GetInstance()->Post(LOG_FATAL, __FILE__, __LINE__,
                    "open() failed: %s", strerror(errno));
            return NULL;
        }
    }
    
    return Http;
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
        /* Shoutcast metadata is embedded in the stream */
        if(LocalBuffer == NULL) {
            LocalBuffer = (char *) __malloc(MetadataFrequency + 4096);
        }
        if((MetadataFrequency * 2) > BufferSize) {
            Log::GetInstance()->Post(LOG_INFO, __FILE__, __LINE__,
                    "Resizing buffer");
            BufferSize = (MetadataFrequency * 2);
            Buffer = (unsigned char *) __realloc(Buffer, BufferSize);
        }

        /* Read in the number of bytes equal to the frequency of the
           metadata updates */
        for(BytesRead = 0; BytesRead < MetadataFrequency; ) {
            Temp = ExtBuffer->ReadNB(LocalBuffer + BytesRead,
                MetadataFrequency - BytesRead);
            if(Temp < 0) {
                Log::GetInstance()->Post(LOG_ERROR, __FILE__, __LINE__,
                        "read() failed: %s", strerror(errno));
                return MAD_FLOW_STOP;
            }
            BytesRead += Temp;
        }

        /* Copy good audio to audio buffer */
        memcpy(Buffer + LeftoverBytes + RetVal, LocalBuffer,
                BytesRead);
        RetVal += BytesRead;

        /* Read the metadata length */
        for(Temp = 0; (Temp >= 0) && (Temp != 1);
                Temp = ExtBuffer->ReadNB(LocalBuffer, 1));
        if(Temp < 0) {
            Log::GetInstance()->Post(LOG_ERROR, __FILE__, __LINE__,
                    "read() failed: %s", strerror(errno));
            return MAD_FLOW_STOP;
        }
        MetadataLength = ((unsigned char) LocalBuffer[0]) * 16;

        /* Read the metadata */
        for(BytesRead = 0; BytesRead != MetadataLength; ) {
            Temp = ExtBuffer->ReadNB(LocalBuffer + BytesRead,
                MetadataLength - BytesRead);
            if(Temp < 0) {
                Log::GetInstance()->Post(LOG_ERROR, __FILE__, __LINE__,
                        "read() failed: %s", strerror(errno));
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
    else {
        /* No metadata, read as normal */
        RetVal = ExtBuffer->ReadNB(Buffer + LeftoverBytes,
                BufferSize - LeftoverBytes);
        if(RetVal < 0) {
            Log::GetInstance()->Post(LOG_ERROR, __FILE__, __LINE__,
                    "read() failed: %s (bytes: %d)", strerror(errno), RetVal);
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
    mad_fixed_t *Resampled[2] = {NULL, NULL};
    static int SamplePos = 0;
    static int LastTime = 0;
    int i;
    enum mad_flow ReturnVal = MAD_FLOW_CONTINUE;
    int NumSamples;
    mad_fixed_t *Samples[2];

    if(Mp3SampleRate != pcm->samplerate) {
        Mp3SampleRate = pcm->samplerate;
        ResampleInit(Mp3SampleRate);
    }
    
    if(Mp3SampleRate != 44100) {
        /* Allocate space for resampled samples */
        Resampled[0] = (mad_fixed_t *) 
                __malloc(sizeof(mad_fixed_t) * pcm->length * 8);
        Resampled[1] = (mad_fixed_t *) 
                __malloc(sizeof(mad_fixed_t) * pcm->length * 8);
        NumSamples = ResampleBlock(pcm->length, pcm->samples[0], Resampled[0]);
        ResampleBlock(pcm->length, pcm->samples[1], Resampled[1]);
        Samples[0] = Resampled[0];
        Samples[1] = Resampled[1];
    }
    else {
        Samples[0] = pcm->samples[0];
        Samples[1] = pcm->samples[1];
        NumSamples = pcm->length;
    }
    
    if(pcm->channels == 1) {
        Samples[1] = Samples[0];
    }
    
    for (i = 0; i < NumSamples; i++) {
        LeftSample = ScaleSample(Samples[0][i]);
        RightSample = ScaleSample(Samples[1][i]);

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

    if(Resampled[0] != NULL) {
        __free(Resampled[0]);
        __free(Resampled[1]);
    }
    
    return ReturnVal;
}

enum mad_flow AudioThread::ErrorCallback(void *ptr, struct mad_stream *stream,
        struct mad_frame *frame) {
    Log::GetInstance()->Post(LOG_WARNING, __FILE__, __LINE__,
            "Decoding error %d - %s", stream->error,
            mad_stream_errorstr(stream));

    return MAD_FLOW_CONTINUE;
}

unsigned int AudioThread::ResampleBlock(unsigned int NumSamples,
        mad_fixed_t const *OrigSamples, mad_fixed_t *NewSamples) {
    /* This function was copied from resample.c in the MAD distribution */
    /* This resampling algorithm is based on a linear interpolation, which is
       not at all the best sounding but is relatively fast and efficient.
      
       A better algorithm would be one that implements a bandlimited
       interpolation. */

    mad_fixed_t const *End, *Begin;

    if(State.ratio == MAD_F_ONE) {
        memcpy(NewSamples, OrigSamples, NumSamples * sizeof(mad_fixed_t));
        return NumSamples;
    }

    End = OrigSamples + NumSamples;
    Begin = NewSamples;

    if(State.step < 0) {
        State.step = mad_f_fracpart(-State.step);

        while(State.step < MAD_F_ONE) {
            if(State.step != 0) {
                *NewSamples++ = State.last +
                        mad_f_mul(*OrigSamples - State.last, State.step);
            }
            else {
                *NewSamples++ = State.last;
            }

            State.step += State.ratio;
            if (((State.step + 0x00000080L) & 0x0fffff00L) == 0) {
                State.step = (State.step + 0x00000080L) & ~0x0fffffffL;
            }
        }

        State.step -= MAD_F_ONE;
    }

    while(End - OrigSamples > 1 + mad_f_intpart(State.step)) {
        OrigSamples += mad_f_intpart(State.step);
        State.step = mad_f_fracpart(State.step);

        if(State.step != 0) {
            *NewSamples++ = *OrigSamples +
                    mad_f_mul(OrigSamples[1] - OrigSamples[0], State.step);
        }
        else {
            *NewSamples++ = *OrigSamples;
        }

        State.step += State.ratio;
        if (((State.step + 0x00000080L) & 0x0fffff00L) == 0) {
            State.step = (State.step + 0x00000080L) & ~0x0fffffffL;
        }
    }

    if(End - OrigSamples == 1 + mad_f_intpart(State.step)) {
        State.last = End[-1];
        State.step = -State.step;
    }
    else {
        State.step -= mad_f_fromint(End - OrigSamples);
    }

    return NewSamples - Begin;
}
    

