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

#ifndef HTTP_H
#define HTTP_H

#include <stdio.h>

FILE *HttpConnect(char *Host, unsigned short Port);
void HttpParseUrl(char *Url, char *Host, unsigned short *Port, char *Path);
int HttpSkipHeader(FILE *fp);
char *HttpUrlEncode(char **string);

#endif /* #ifndef HTTP_H */
