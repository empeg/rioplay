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
#include "Mp3Decoder.hh"
#include "Tag.h"
#include "Http.hh"
#include "Player.h"
#include "Log.hh"
#include "BufferClass.hh"
#include "Globals.hh"
#include "MemAlloc.hh"

extern int errno;

Mp3Decoder *MadCallbackDecoder;
enum mad_flow InputCallbackJump(void *ptr, struct mad_stream *stream);
enum mad_flow OutputCallbackJump(void *ptr, struct mad_header const *header, struct mad_pcm *pcm);
enum mad_flow ErrorCallbackJump(void *ptr, struct mad_stream *stream, struct mad_frame *frame);

Mp3Decoder::Mp3Decoder(int inInputFD, AudioOutputDevice *inAudioOut,
        InputSource *inPList) {
    /* Initialize class variables */
    SongFD = -1;
    CurrentTime.seconds = 0;
    CurrentTime.fraction = 0;
    MetadataFrequency = 0;
    FirstRun = 0;
    LocalBuffer = NULL;
    BufferSize = BUFFER_SIZE;
    
    Buffer = (unsigned char *) __malloc(BufferSize);
    
    AudioOut = inAudioOut;
    PList = inPList;
    SongFD = inInputFD;
    MadCallbackDecoder = this;
    
    /* Initialize decoder */    
    mad_decoder_init(&MadDecoder, NULL, InputCallbackJump,
            NULL, NULL, OutputCallbackJump, ErrorCallbackJump, NULL);
}    

Mp3Decoder::~Mp3Decoder(void) {
    pthread_mutex_lock(&ClassMutex);
    /* Signal decoding thread to stop playback */
    Stop = true;
    pthread_mutex_unlock(&ClassMutex);
    
    /* Wait until decoding thread exits */
    pthread_join(ThreadHandle, NULL);

    /* Clean up member variables */    
    __free(Buffer);
    
    if(LocalBuffer != NULL) {
        __free(LocalBuffer);
    }
}

void *Mp3Decoder::ThreadMain(void *arg) {
    /* Set up external buffer thread */
    ExtBuffer = new BufferClass(SongFD, 81920);
    
    /* Fill the buffer before we start playback */
    ExtBuffer->Prebuffer();

    /* Set time to 0 */
    CurrentTime.seconds = 0;
    CurrentTime.fraction = 0;

    /* Prepare input function */
    FirstRun = 0;
    if(LocalBuffer != NULL) {
        __free(LocalBuffer);
        LocalBuffer = NULL;
    }

    /* Set up output device */
    AudioOut->SetBitsPerSample(MAD_F_FRACBITS);
    
    /* Decode and play the audio */
    mad_decoder_run(&MadDecoder, MAD_DECODER_MODE_SYNC);

    Log::GetInstance()->Post(LOG_INFO, __FILE__, __LINE__,
            "Decoder finished");

    /* Close Audio file */
    delete ExtBuffer;
    ExtBuffer = NULL;

    Globals::Status.SetTime(0, 0);
    Globals::Display.Update(&Globals::Status);
    
    /* Signal our input source that we're done decoding */
    if(Reason != REASON_STOP_REQUESTED) {
        /* If the stop was requested then we don't want to signal */
        PList->DecoderFinished();
    }
    
    return NULL;
}

enum mad_flow Mp3Decoder::InputCallback(void *ptr, struct mad_stream *stream) {
    int LeftoverBytes;
    int RetVal = 0;
    int MetadataLength, BytesRead, Temp;
    Tag TrackTag;

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
                Reason = REASON_READ_FAILED;
                return MAD_FLOW_BREAK;
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
            Reason = REASON_READ_FAILED;
            return MAD_FLOW_BREAK;
        }
        MetadataLength = ((unsigned char) LocalBuffer[0]) * 16;

        /* Read the metadata */
        for(BytesRead = 0; BytesRead != MetadataLength; ) {
            Temp = ExtBuffer->ReadNB(LocalBuffer + BytesRead,
                MetadataLength - BytesRead);
            if(Temp < 0) {
                Log::GetInstance()->Post(LOG_ERROR, __FILE__, __LINE__,
                        "read() failed: %s", strerror(errno));
                Reason = REASON_READ_FAILED;
                return MAD_FLOW_BREAK;
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

            TrackTag = PList->SetMetadata(LocalBuffer, MetadataLength);
            Globals::Status.SetAttribs(TrackTag);
            Globals::Display.Update(&Globals::Status);
        }
    }
    else {
        /* No metadata, read as normal */
        RetVal = ExtBuffer->ReadNB(Buffer + LeftoverBytes,
                BufferSize - LeftoverBytes);
        if(RetVal < 0) {
            Log::GetInstance()->Post(LOG_ERROR, __FILE__, __LINE__,
                    "read() failed: %s (bytes: %d)", strerror(errno), RetVal);
            Reason = REASON_READ_FAILED;
            return MAD_FLOW_BREAK;
        }
    }

    /* Signal MAD that buffer is ready */
    mad_stream_buffer(stream, Buffer, RetVal + LeftoverBytes);

    /* Successful completion */
    return MAD_FLOW_CONTINUE;
}

enum mad_flow Mp3Decoder::OutputCallback(void *ptr,
        struct mad_header const *header, struct mad_pcm *pcm) {
    static int LastTime = 0;
    mad_fixed_t *Left, *Right;

    /* Check for stop command */
    pthread_mutex_lock(&ClassMutex);
    if(Stop == true) {
        pthread_mutex_unlock(&ClassMutex);
        Reason = REASON_STOP_REQUESTED;
        return MAD_FLOW_STOP;
    }
    pthread_mutex_unlock(&ClassMutex);
    
    /* Configure output device to correct sample rate */
    AudioOut->SetSampleRate(pcm->samplerate);
    
    Left = pcm->samples[0];
    Right = pcm->samples[1];
    
    if(pcm->channels == 1) {
        Right = Left;
    }
    
    /* Check for pause */
    pthread_mutex_lock(&ClassMutex);
    if(Paused == true) {
        pthread_cond_wait(&ClassCondition, &ClassMutex);
    }
    pthread_mutex_unlock(&ClassMutex);

    /* Play the decoded samples */
    AudioOut->Play(Left, Right, pcm->length);

    /* Lock */
    pthread_mutex_lock(&ClassMutex);

    /* Update time display */
    mad_timer_add(&CurrentTime, header->duration);
    if (LastTime != CurrentTime.seconds) {
        Globals::Status.SetTime(CurrentTime.seconds / 60,
                CurrentTime.seconds % 60);
        LastTime = CurrentTime.seconds;

        /* Signal the display thread that the time has changed */
        Globals::Display.Update(&Globals::Status);
    }

    /* Unlock */
    pthread_mutex_unlock(&ClassMutex);

    /* Force a yield so display thread can run */
    sched_yield();
    
    return MAD_FLOW_CONTINUE;
}

enum mad_flow Mp3Decoder::ErrorCallback(void *ptr, struct mad_stream *stream,
        struct mad_frame *frame) {
    Log::GetInstance()->Post(LOG_WARNING, __FILE__, __LINE__,
            "MP3 Decoding error %d - %s", stream->error,
            mad_stream_errorstr(stream));

    return MAD_FLOW_CONTINUE;
}

void Mp3Decoder::SetMetadataFrequency(int Freq) {
    MetadataFrequency = Freq;
}

enum mad_flow InputCallbackJump(void *ptr, struct mad_stream *stream) {
    return MadCallbackDecoder->InputCallback(ptr, stream);
}

enum mad_flow OutputCallbackJump(void *ptr, struct mad_header const *header,
        struct mad_pcm *pcm) {
    return MadCallbackDecoder->OutputCallback(ptr, header, pcm);
}

enum mad_flow ErrorCallbackJump(void *ptr, struct mad_stream *stream,
        struct mad_frame *frame) {
    return MadCallbackDecoder->ErrorCallback(ptr, stream, frame);
}
