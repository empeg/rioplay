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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/soundcard.h>
#include <errno.h>
#include <sys/ioctl.h>
#include "EmpegAudio.hh"
#include "Log.hh"
#include "MemAlloc.hh"

extern int errno;

EmpegAudio::EmpegAudio(void) {
        #define EMPEG_MIXER_MAGIC 'm'
        #define EMPEG_MIXER_WRITE_SOURCE _IOW(EMPEG_MIXER_MAGIC, 0, int)
        #define EMPEG_MIXER_WRITE_FLAGS _IOW(EMPEG_MIXER_MAGIC, 1, int)
        #define EMPEG_MIXER_SET_SAM _IOW(EMPEG_MIXER_MAGIC, 15, int)

    int source, soft_audio_mute, mute;

    /* Initialize member variables */
    SamplePos = 0;
    
    /* Open the audio device for output */
    AudioFD = open("/dev/audio", O_WRONLY);
    if (AudioFD < 0) {
        Log::GetInstance()->Post(LOG_FATAL, __FILE__, __LINE__,
                "Cannot open audio device");
        return;
    }
    
    /* Open mixer device for output */
    MixerFD = open("/dev/mixer", O_WRONLY);
    if(MixerFD < 0) {
        Log::GetInstance()->Post(LOG_ERROR, __FILE__, __LINE__,
                "Could not open mixer device");
        return;
    }

    /* This particular bit of code is necessary to prepare the Empeg's mixer
     * 	to play audio.  Without running this, no audio will be heard. */    

	source = SOUND_MASK_PCM;
	if (ioctl(MixerFD, EMPEG_MIXER_WRITE_SOURCE, &source) == -1) {
	        Log::GetInstance()->Post(LOG_FATAL, __FILE__, __LINE__,
			                      "ioctl(EMPEG_MIXER_WRITE_SOURCE");
	}

	soft_audio_mute = 0;
	if (ioctl(MixerFD, EMPEG_MIXER_SET_SAM, &soft_audio_mute) == -1) {
	        Log::GetInstance()->Post(LOG_FATAL, __FILE__, __LINE__,
			                      "ioctl(EMPEG_MIXER_SET_SAM");
	}

	mute = 0;
	if (ioctl(MixerFD, EMPEG_MIXER_WRITE_FLAGS, &mute) == -1) {
	        Log::GetInstance()->Post(LOG_FATAL, __FILE__, __LINE__,
			                      "ioctl(EMPEG_MIXER_WRITE_FLAGS");
	}
 
    /* Set volume to an acceptable level */    
    SetVolume(45);
    
    /* Default the sample rate and bits/sample */
    SetSampleRate(44100);
    BitsPerSample = 16;
}

EmpegAudio::~EmpegAudio(void) {
    close(AudioFD);
    close(MixerFD);
}
