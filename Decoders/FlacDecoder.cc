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
#include "FlacDecoder.hh"
#include "Globals.hh"
#include "MemAlloc.hh"

extern int errno;

FlacDecoder *FlacCallbackDecoder;
FLAC__StreamDecoderReadStatus InputCallbackJump(const FLAC__StreamDecoder *decoder, FLAC__byte buffer[], unsigned *bytes, void *client_data);
FLAC__StreamDecoderWriteStatus OutputCallbackJump(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *client_data);
void ErrorCallbackJump(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data);
void MetadataCallbackJump(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data);

FlacDecoder::FlacDecoder(int inInputFD, InputSource *inPList) {
    /* Initialize class variables */
    MetadataFrequency = 0;
    BufferSize = BUFFER_SIZE;
    ExtBuffer = NULL;
    
    Buffer = (unsigned char *) __malloc(BufferSize);
    
    PList = inPList;
    SongFD = inInputFD;
    FlacCallbackDecoder = this;
    
    /* Initialize decoder */
    FLACDecoder = FLAC__stream_decoder_new();
    
    FLAC__stream_decoder_set_read_callback(FLACDecoder, &InputCallbackJump);
    FLAC__stream_decoder_set_write_callback(FLACDecoder, &OutputCallbackJump);
    FLAC__stream_decoder_set_error_callback(FLACDecoder, &ErrorCallbackJump);
    FLAC__stream_decoder_set_metadata_callback(FLACDecoder, &MetadataCallbackJump);
    
    FLAC__stream_decoder_init(FLACDecoder);
}    

FlacDecoder::~FlacDecoder(void) {
    pthread_mutex_lock(&ClassMutex);
    /* Signal decoding thread to stop playback */
    Stop = true;
    pthread_mutex_unlock(&ClassMutex);
    
    /* Wait until decoding thread exits */
    pthread_join(ThreadHandle, NULL);

    __free(Buffer);
    
    FLAC__stream_decoder_delete(FLACDecoder);
}

void *FlacDecoder::ThreadMain(void *arg) {
    /* Set up external buffer thread */
    ExtBuffer = new BufferClass(SongFD, 163680);
    
    /* Fill the buffer before we start playback */
    ExtBuffer->Prebuffer();

    /* Decode the stream */
    FLAC__stream_decoder_process_whole_stream(FLACDecoder);
    
    /* Clean up */
    FLAC__stream_decoder_finish(FLACDecoder);
    
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

FLAC__StreamDecoderReadStatus FlacDecoder::InputCallback(const FLAC__StreamDecoder *decoder,
        FLAC__byte buffer[], unsigned *bytes, void *client_data) {
    int RetVal;
    
    /* Read up to *bytes from buffer */
    RetVal = ExtBuffer->ReadNB(buffer, *bytes);
    
    if(RetVal < 0) {
        /* Error occured while reading from buffer */
        Log::GetInstance()->Post(LOG_ERROR, __FILE__, __LINE__,
                "read() failed: %s (bytes: %d)", strerror(errno), RetVal);
        Reason = REASON_READ_FAILED;
        return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
    }

    /* Indicate the number of bytes being returned */
    *bytes = RetVal;

    /* Successful completion */
    return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
}

FLAC__StreamDecoderWriteStatus FlacDecoder::OutputCallback(
        const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame,
        const FLAC__int32 * const buffer[], void *client_data) {
    const mad_fixed_t *Left, *Right;
    static int LastTime = 0;
    int TempTime;

    /* Check for stop command */
    pthread_mutex_lock(&ClassMutex);
    if(Stop == true) {
        pthread_mutex_unlock(&ClassMutex);
        Globals::AudioOut->Flush();
        Reason = REASON_STOP_REQUESTED;
        return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
    }
    pthread_mutex_unlock(&ClassMutex);
    
    /* Configure output device to correct sample rate */
    Globals::AudioOut->SetSampleRate(frame->header.sample_rate);
    Globals::AudioOut->SetBitsPerSample(frame->header.bits_per_sample);

    Left = (mad_fixed_t *) buffer[0];
    Right = (mad_fixed_t *) buffer[1];
    
    if(frame->header.channels == 1) {
        Right = Left;
    }
    
    /* Check for pause */
    pthread_mutex_lock(&ClassMutex);
    if(Paused == true) {
        pthread_cond_wait(&ClassCondition, &ClassMutex);
    }
    pthread_mutex_unlock(&ClassMutex);

    /* Play the decoded samples */
    Globals::AudioOut->Play(Left, Right, frame->header.blocksize);

    pthread_mutex_lock(&ClassMutex);
    
    /* Update time display */
    TempTime = frame->header.number.frame_number / frame->header.sample_rate;
    if (LastTime != TempTime) {
        Globals::Status.SetTime(TempTime / 60,
                TempTime % 60);
        LastTime = TempTime;

        /* Signal the display thread that the time has changed */
        Globals::Display.Update(&Globals::Status);
    }
    
    pthread_mutex_unlock(&ClassMutex);

    /* Force a yield so display thread can run */
    sched_yield();
    
    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void FlacDecoder::ErrorCallback(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data) {
    switch(status) {
        case FLAC__STREAM_DECODER_ERROR_STATUS_LOST_SYNC:
            Log::GetInstance()->Post(LOG_WARNING, __FILE__, __LINE__,
                    "FLAC Decoding error - lost synchronization");
            break;
            
        case FLAC__STREAM_DECODER_ERROR_STATUS_BAD_HEADER:
            Log::GetInstance()->Post(LOG_WARNING, __FILE__, __LINE__,
                    "FLAC Decoding error - bad header");
            break;
            
        case FLAC__STREAM_DECODER_ERROR_STATUS_FRAME_CRC_MISMATCH:
            Log::GetInstance()->Post(LOG_WARNING, __FILE__, __LINE__,
                    "FLAC Decoding error - CRC mismatch");
            break;
        
        default:
            Log::GetInstance()->Post(LOG_WARNING, __FILE__, __LINE__,
                    "FLAC Decoding error - unknown error %d", status);
            break;
    }
}

void FlacDecoder::MetadataCallback(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data) {
}


FLAC__StreamDecoderReadStatus InputCallbackJump(
        const FLAC__StreamDecoder *decoder, FLAC__byte buffer[],
        unsigned *bytes, void *client_data) {
    return FlacCallbackDecoder->InputCallback(decoder, buffer, bytes,
            client_data);
}

FLAC__StreamDecoderWriteStatus OutputCallbackJump(
        const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame,
        const FLAC__int32 * const buffer[], void *client_data) {
    return FlacCallbackDecoder->OutputCallback(decoder, frame, buffer,
            client_data);
}

void ErrorCallbackJump(const FLAC__StreamDecoder *decoder,
        FLAC__StreamDecoderErrorStatus status, void *client_data) {
    FlacCallbackDecoder->ErrorCallback(decoder, status, client_data);
}

void MetadataCallbackJump(const FLAC__StreamDecoder *decoder,
        const FLAC__StreamMetadata *metadata, void *client_data) {
    FlacCallbackDecoder->MetadataCallback(decoder, metadata, client_data);
}
