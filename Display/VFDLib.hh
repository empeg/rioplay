// vfdlib - a graphics api library for empeg userland apps
// Copyright (C) 2002  Richard Kirkby
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// 
// Send comments, suggestions, bug reports to rkirkby (at) cs.waikato.ac.nz
//
// Revisions:
//   Converted to a C++ class - David Flowerday, 08-22-02

#ifndef VFDLIB_HH
#define VFDLIB_HH

#define VFD_FONT_SMALL		0
#define VFD_FONT_MEDIUM		1
#define VFD_FONT_TIME		2

#define VFD_DEFAULT_FONT VFD_FONT_MEDIUM
#define VFD_HEIGHT		64
#define VFD_WIDTH               128
#define VFD_BYTES_PER_SCANLINE  64

#define VFDSHADE_BLACK          0x00
#define VFDSHADE_DIM            0x01
#define VFDSHADE_MEDIUM         0x02
#define VFDSHADE_BRIGHT         0x03

#define BITMAP_1BPP             0
#define BITMAP_2BPP             1
#define BITMAP_4BPP             2

#define NUM_FONT_SLOTS   5

typedef struct {
  short offset;
  char width;
} CharInfo;

typedef struct {
  CharInfo *cInfo;
  char *fBitmap;
  int firstIndex;
  int numOfChars;
} FontInfo;

class VFDLib {
public:
    /* Constructors & desctructor */
    VFDLib(void);
    ~VFDLib(void);

    /* Set buffer */
    void setBuffer(char *inDisplayMemMap);
        
    /* Clip area settings */
    void setClipArea(int xLeft, int yTop, int xRight, int yBottom);
    void clear(char shade);
    
    /* Points */
    void drawPointClipped(int xPos, int yPos, char shade);
    void drawPointUnclipped(int xPos, int yPos, char shade);
    
    /* Lines */
    void drawLineHorizClipped(int yPos, int xLeft, int xRight, char shade);
    void drawLineHorizUnclipped(int yPos, int xLeft, int length, char shade);
    void drawLineVertClipped(int xPos, int yTop, int yBottom, char shade);
    void drawLineVertUnclipped(int xPos, int yTop, int length, char shade);
    void drawLineClipped(int x1, int y1, int x2, int y2, char shade);
    void drawLineUnclipped(int x1, int y1, int x2, int y2, char shade);

    /* Rectangles */
    void drawOutlineRectangleClipped(int xLeft, int yTop, int xRight,
            int yBottom, char shade);
    void drawOutlineRectangleUnclipped(int xLeft, int yTop, int xWidth,
            int yHeight, char shade);
    void drawSolidRectangleClipped(int xLeft, int yTop, int xRight,
            int yBottom, char shade);
    void drawSolidRectangleUnclipped(int xLeft, int yTop, int xWidth,
            int yHeight, char shade);
    void invertRectangleClipped(int xLeft, int yTop, int xRight, int yBottom);
    void invertRectangleUnclipped(int xLeft, int yTop, int xWidth, int yHeight);

    /* Ellipses */
    void drawOutlineEllipseClipped(int cX, int cY, int halfWidth,
            int halfHeight, char shade);
    void drawOutlineEllipseUnclipped(int cX, int cY, int halfWidth,
            int halfHeight, char shade);
    void drawSolidEllipseClipped(int cX, int cY, int halfWidth,
            int halfHeight, char shade);
    void drawSolidEllipseUnclipped(int cX, int cY, int halfWidth,
            int halfHeight, char shade);

    /* Polygons */
    void drawOutlinePolygonClipped(int *coordsData, int numPoints, char shade);
    void drawOutlinePolygonUnclipped(int *coordsData, int numPoints,
            char shade);
    void drawSolidPolygon(int *coordsData, int numPoints, char shade);
    
    /* Fonts */
    int registerFont(char *bfFileName, int fontSlot);
    void unregisterFont(int fontSlot);
    void unregisterAllFonts();
    int getTextHeight(int fontSlot);
    int getTextWidth(char *string, int fontSlot);
    int getStringIndexOfCutoffWidth(char *string, int fontSlot, int width);
    void drawText(char *string, int xLeft, int yTop, int fontSlot, char shade);
    
    /* Bitmaps */
    void drawBitmap(unsigned char *bitmap, int destX, int destY,
            int sourceX, int sourceY, int width, int height,
            signed char shade, int isTransparent);

private:
    void drawPixel4WaySymmetricClipped(int cX, int cY,
            int x, int y, char shade);
    void drawPixel4WaySymmetricUnclipped(int cX, int cY,
            int x, int y, char shade);
    void drawLineHMaxClipped(int x0, int y0, int dx, int dy,
            int d, int xMax, int yMax, char shade);
    void drawLineVMaxClipped(int x0, int y0, int dx, int dy,
            int d, int xMax, int yMax, char shade);
    void drawLineHMaxUnclipped(int x0, int y0, int dx, int dy, char shade);
    void drawLineVMaxUnclipped(int x0, int y0, int dx, int dy, char shade);
    void drawBitmap1BPP(unsigned char *bitmap,
            int bitmapWidth, int bitmapHeight, int destX, int destY,
            int sourceX, int sourceY, int width, int height,
            signed char shade, int isTransparent);
    void drawBitmap2BPP(unsigned char *bitmap, int bitmapWidth,
            int bitmapHeight, int destX, int destY, int sourceX, int sourceY,
            int width, int height, signed char shade, int isTransparent);
    void drawBitmap4BPP(unsigned char *bitmap, int bitmapWidth,
            int bitmapHeight, int destX, int destY, int sourceX, int sourceY,
            int width, int height, signed char shade, int isTransparent);
    
    int g_clipXLeft;
    int g_clipYTop;
    int g_clipXRight;
    int g_clipYBottom;
    FontInfo g_fontRegistry[NUM_FONT_SLOTS];
    char *DisplayMemMap;
};

#endif // #ifndef VFDLIB_HH
