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

#ifndef MADCALLBACKS_H
#define MADCALLBACKS_H

#include "mad.h"

enum mad_flow InputCallbackJump(void *ptr, struct mad_stream *stream);
enum mad_flow OutputCallbackJump(void *ptr, struct mad_header const *header, struct mad_pcm *pcm);
enum mad_flow ErrorCallbackJump(void *ptr, struct mad_stream *stream, struct mad_frame *frame);

#endif
