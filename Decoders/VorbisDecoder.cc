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
#include "Tag.h"
#include "Player.h"
#include "Log.hh"
#include "BufferClass.hh"
#include "VorbisDecoder.hh"
#include "Globals.hh"
#include "MemAlloc.hh"

extern int errno;

VorbisDecoder *VorbisDecoderCallback;
size_t ReadCallbackJump(void *ptr, size_t size, size_t nmemb, void *datasource);
int SeekCallbackJump(void *datasource, ogg_int64_t offset, int whence);
int CloseCallbackJump(void *datasource);

VorbisDecoder::VorbisDecoder(int inInputFD, AudioOutputDevice *inAudioOut,
        InputSource *inPList) {
    /* Initialize class variables */
    BufferSize = BUFFER_SIZE;
    ExtBuffer = NULL;
    VorbisDecoderCallback = this;
    
    Buffer = (unsigned char *) __malloc(BufferSize);
    
    AudioOut = inAudioOut;
    PList = inPList;
    SongFD = inInputFD;
    
    /* Initialize decoder */
    Callbacks.read_func = ReadCallbackJump;
    Callbacks.seek_func = SeekCallbackJump;
    Callbacks.close_func = CloseCallbackJump;
    Callbacks.tell_func = NULL;

    /* Set up external buffer thread */
    printf("Creating new buffer\n");
    ExtBuffer = new BufferClass(SongFD, 163680);
    
    /* Fill the buffer before we start playback */
    printf("Prebuffering\n");
    ExtBuffer->Prebuffer();
    printf("Done\n");

    printf("@@@@@ VorbisDecoder init\n");
        
    if (ov_open_callbacks(stdin, &vf, NULL, 0, Callbacks) < 0) {
        fprintf(stderr, "Input does not appear to be an Ogg bitstream.\n");
    }
    
    printf("@@@@@ Finished opening\n");
}    

VorbisDecoder::~VorbisDecoder(void) {
    pthread_mutex_lock(&ClassMutex);
    /* Signal decoding thread to stop playback */
    Stop = true;
    pthread_mutex_unlock(&ClassMutex);
    
    /* Wait until decoding thread exits */
    pthread_join(ThreadHandle, NULL);

    /* cleanup */
    ov_clear(&vf);
    
    __free(Buffer);
}

void *VorbisDecoder::ThreadMain(void *arg) {
    /* Local variables */
    int eof = 0;
    int current_section;
    static int LastTime = 0;
    int TempTime;
    bool TempStop;
    
    /* Throw the comments plus a few lines about the bitstream we're
       decoding */
    char **ptr = ov_comment(&vf, -1)->user_comments;
    vorbis_info *vi = ov_info(&vf, -1);
    while (*ptr) {
        fprintf(stderr, "%s\n", *ptr);
        ++ptr;
    }
    fprintf(stderr, "\nBitstream is %d channel, %ldHz\n", vi->channels, vi->rate);
    fprintf(stderr, "\nDecoded length: %ld samples\n",
            (long)ov_pcm_total(&vf, -1));
    fprintf(stderr, "Encoded by: %s\n\n", ov_comment(&vf, -1)->vendor);

    /* Main decoding loop */
    while (!eof) {
        long ret = ov_read(&vf, pcmout, sizeof(pcmout), &current_section);
        pthread_mutex_lock(&ClassMutex);
        TempStop = Stop;
        pthread_mutex_unlock(&ClassMutex);
        if (ret == 0) {
            /* EOF */
            eof = 1;
        }
        else if (ret < 0) {
            /* error in the stream.  Not a problem, just reporting it in
            case we (the app) cares.  In this case, we don't. */
        }
        else if(TempStop == true) {
            /* Stop command received */
            Reason = REASON_STOP_REQUESTED;
            AudioOut->Flush();
            eof = 1;
        }
        else {
            /* Configure output device to correct sample rate */
            AudioOut->SetSampleRate(ov_info(&vf, -1)->rate);

            /* Play the decoded samples */
            AudioOut->Play(pcmout, ret);

            /* Update time display */
            TempTime = ov_pcm_tell(&vf) / ov_info(&vf, -1)->rate;
            if (LastTime != TempTime) {
                Globals::Status.SetTime(TempTime / 60, TempTime % 60);
                LastTime = TempTime;

                /* Signal the display thread that the time has changed */
                Globals::Display.Update(&Globals::Status);
            }
        }
    }

    return NULL;
}

size_t VorbisDecoder::ReadCallback(void *ptr, size_t size, size_t nmemb,
        void *datasource) {
    int ReturnVal;
    
    while((ReturnVal = ExtBuffer->ReadNB(ptr, nmemb * size)) == 0);
    
    if(ReturnVal == -1) {
        /* Buffer is empty and won't be refilled, so signal Vorbis decoder
           that this is the end of the file */
        return 0;
    }
    return ReturnVal;
}

size_t ReadCallbackJump(void *ptr, size_t size, size_t nmemb, void *datasource) {
    return VorbisDecoderCallback->ReadCallback(ptr, size, nmemb, datasource);
}

int SeekCallbackJump(void *datasource, ogg_int64_t offset, int whence) {
    return -1;
}

int CloseCallbackJump(void *datasource) {
    return 0;
}
