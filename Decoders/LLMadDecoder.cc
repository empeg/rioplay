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
#include "LLMadDecoder.hh"
#include "Tag.h"
#include "Player.h"
#include "Log.hh"
#include "BufferClass.hh"
#include "Globals.hh"
#include "MemAlloc.hh"

#define INPUT_BUFFER_SIZE (5 * 8192)

extern int errno;

LLMadDecoder::LLMadDecoder(int inInputFD, InputSource *inPList) {
    /* Initialize class variables */
    CurrentTime.seconds = 0;
    CurrentTime.fraction = 0;
    
    PList = inPList;
    SongFD = inInputFD;
    
    /* Initialize MAD structures */
    mad_stream_init(&Stream);
    mad_frame_init(&Frame);
    mad_synth_init(&Synth);
}    

LLMadDecoder::~LLMadDecoder(void) {
    pthread_mutex_lock(&ClassMutex);
    /* Signal decoding thread to stop playback */
    Stop = true;
    pthread_mutex_unlock(&ClassMutex);
    
    /* Wait until decoding thread exits */
    pthread_join(ThreadHandle, NULL);
    
	/* Mad is no longer used, the structures that were initialized must
     * now be cleared.
	 */
	mad_synth_finish(&Synth);
	mad_frame_finish(&Frame);
	mad_stream_finish(&Stream);
}

void *LLMadDecoder::ThreadMain(void *arg) {
	unsigned char		InputBuffer[INPUT_BUFFER_SIZE];
	unsigned long		FrameCount=0;
    mad_fixed_t *Left, *Right;
    int LastTime = 0;
    
    /* Set up external buffer thread */
    ExtBuffer = new BufferClass(SongFD, 81920);
    
    /* Fill the buffer before we start playback */
    ExtBuffer->Prebuffer();

    /* Set time to 0 */
    CurrentTime.seconds = 0;
    CurrentTime.fraction = 0;

    /* Set up output device */
    Globals::AudioOut->SetBitsPerSample(MAD_F_FRACBITS);

    /* Main decoding loop */    
    while(1) {
		/* The input bucket must be filled if it becomes empty or if
		 * it's the first execution of the loop.
		 */
		if(Stream.buffer==NULL || Stream.error==MAD_ERROR_BUFLEN)
		{
			ssize_t			ReadSize,
							Remaining;
			unsigned char	*ReadStart;

			/* {1} libmad may not consume all bytes of the input
			 * buffer. If the last frame in the buffer is not wholly
			 * contained by it, then that frame's start is pointed by
			 * the next_frame member of the Stream structure. This
			 * common situation occurs when mad_frame_decode() fails,
			 * sets the stream error code to MAD_ERROR_BUFLEN, and
			 * sets the next_frame pointer to a non NULL value. (See
			 * also the comment marked {2} bellow.)
			 *
			 * When this occurs, the remaining unused bytes must be
			 * put back at the beginning of the buffer and taken in
			 * account before refilling the buffer. This means that
			 * the input buffer must be large enough to hold a whole
			 * frame at the highest observable bit-rate (currently 448
			 * kb/s). XXX=XXX Is 2016 bytes the size of the largest
			 * frame? (448000*(1152/32000))/8
			 */
			if(Stream.next_frame!=NULL)
			{
				Remaining=Stream.bufend-Stream.next_frame;
				memmove(InputBuffer,Stream.next_frame,Remaining);
				ReadStart=InputBuffer+Remaining;
				ReadSize=INPUT_BUFFER_SIZE-Remaining;
			}
			else {
				ReadSize=INPUT_BUFFER_SIZE;
                ReadStart=InputBuffer;
				Remaining=0;
            }
			
			/* Fill-in the buffer. If an error occurs print a message
			 * and leave the decoding loop. If the end of stream is
			 * reached we also leave the loop but the return status is
			 * left untouched.
			 */
			ReadSize = ExtBuffer->ReadNB(ReadStart, ReadSize);
			if(ReadSize < 0)
			{
                Log::GetInstance()->Post(LOG_INFO, __FILE__, __LINE__,
                    "End of file detected");
                Globals::AudioOut->Flush();
                Reason = REASON_READ_FAILED;
                break;
			}

			/* Pipe the new buffer content to libmad's stream decoder
             * facility.
			 */
			mad_stream_buffer(&Stream,InputBuffer,ReadSize+Remaining);
			Stream.error = MAD_ERROR_NONE;
		}

		/* Decode the next mpeg frame. The streams is read from the
		 * buffer, its constituents are break down and stored the the
		 * Frame structure, ready for examination/alteration or PCM
		 * synthesis. Decoding options are carried in the Frame
		 * structure from the Stream structure.
		 *
		 * Error handling: mad_frame_decode() returns a non zero value
		 * when an error occurs. The error condition can be checked in
		 * the error member of the Stream structure. A mad error is
		 * recoverable or fatal, the error status is checked with the
		 * MAD_RECOVERABLE macro.
		 *
		 * {2} When a fatal error is encountered all decoding
		 * activities shall be stopped, except when a MAD_ERROR_BUFLEN
		 * is signaled. This condition means that the
		 * mad_frame_decode() function needs more input to achieve
		 * it's work. One shall refill the buffer and repeat the
		 * mad_frame_decode() call. Some bytes may be left unused at
		 * the end of the buffer if those bytes forms an incomplete
		 * frame. Before refilling, the remainign bytes must be moved
		 * to the begining of the buffer and used for input for the
		 * next mad_frame_decode() invocation. (See the comments marked
		 * {1} earlier for more details.)
		 *
		 * Recoverable errors are caused by malformed bit-streams, in
		 * this case one can call again mad_frame_decode() in order to
		 * skip the faulty part and re-sync to the next frame.
		 */
		if(mad_frame_decode(&Frame,&Stream)) {
			if(MAD_RECOVERABLE(Stream.error))
			{
                Log::GetInstance()->Post(LOG_WARNING, __FILE__, __LINE__,
                    "Recoverable MP3 frame error %d - %s", Stream.error,
                    mad_stream_errorstr(&Stream));
				continue;
			}
			else {
				if(Stream.error==MAD_ERROR_BUFLEN) {
					continue;
                }
				else
				{
                    Log::GetInstance()->Post(LOG_ERROR, __FILE__, __LINE__,
                        "Unrecoverable MP3 frame error %d - %s", Stream.error,
                        mad_stream_errorstr(&Stream));
                    Globals::AudioOut->Flush();
                    Reason = REASON_READ_FAILED;
					break;
				}
            }
        }

        /* Check for stop command */
        pthread_mutex_lock(&ClassMutex);
        if(Stop == true) {
            pthread_mutex_unlock(&ClassMutex);
            Globals::AudioOut->Flush();
            Reason = REASON_STOP_REQUESTED;
            break;
        }
        pthread_mutex_unlock(&ClassMutex);
    
        /* Check for pause */
        pthread_mutex_lock(&ClassMutex);
        if(Paused == true) {
            pthread_cond_wait(&ClassCondition, &ClassMutex);
        }
        pthread_mutex_unlock(&ClassMutex);
    
		/* Accounting. The computed frame duration is in the frame
		 * header structure. It is expressed as a fixed point number
		 * whole data type is mad_timer_t. It is different from the
		 * samples fixed point format and unlike it, it can't directly
		 * be added or substracted. The timer module provides several
		 * functions to operate on such numbers. Be careful there, as
		 * some functions of mad's timer module receive some of their
		 * mad_timer_t arguments by value!
		 */
		FrameCount++;
				
		/* Once decoded the frame is synthesized to PCM samples. No errors
		 * are reported by mad_synth_frame();
		 */
		mad_synth_frame(&Synth,&Frame);

		/* Synthesized samples must be converted from mad's fixed
		 * point number to the consumer format. Here we use unsigned
		 * 16 bit big endian integers on two channels. Integer samples
		 * are temporarily stored in a buffer that is flushed when
		 * full.
		 */
        Left = Synth.pcm.samples[0];
        Right = Synth.pcm.samples[1];

        if(Synth.pcm.channels == 1) {
            Right = Left;
        }

        /* Play the decoded samples */
        Globals::AudioOut->Play(Left, Right, Synth.pcm.length);
        
        /* Lock */
        pthread_mutex_lock(&ClassMutex);

        /* Update time display */
        mad_timer_add(&CurrentTime, Frame.header.duration);
        if (LastTime != CurrentTime.seconds) {
            Globals::Status->SetTime(CurrentTime.seconds / 60,
                    CurrentTime.seconds % 60);
            LastTime = CurrentTime.seconds;

            /* Signal the display thread that the time has changed */
            Globals::Display->Update(Globals::Status);
        }

        /* Unlock */
        pthread_mutex_unlock(&ClassMutex);
	}
    

    Log::GetInstance()->Post(LOG_INFO, __FILE__, __LINE__,
            "Decoder finished");

    /* Close Audio file */
    delete ExtBuffer;
    ExtBuffer = NULL;

    Globals::Status->SetTime(0, 0);
    Globals::Display->Update(Globals::Status);
    
    /* Signal our input source that we're done decoding */
    if(Reason != REASON_STOP_REQUESTED) {
        /* If the stop was requested then we don't want to signal */
        PList->DecoderFinished();
    }
    
    return NULL;
}
