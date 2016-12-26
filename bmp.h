//
//  bmp.h
//  SIMDLAB
//
//  Created by Nancy Fan on 12/26/16.
//  Copyright © 2016 Nancy Fan. All rights reserved.
//

#ifndef bmp_h
#define bmp_h

typedef unsigned short int WORD;
typedef unsigned int DWORD;
typedef int LONG;
typedef unsigned char BYTE;

#pragma pack(1)

typedef struct tagBITMAPFILEHEADER
{
    WORD bfType;
    DWORD bfSize;
    WORD bfReserved1;
    WORD bfReserved2;
    DWORD bfOffBits;
    
    tagBITMAPFILEHEADER(){
        bfType = 19778;
        bfSize = 6220854;
        bfReserved1 = 0;
        bfReserved2 = 0;
        bfOffBits = 54;
    }
}t_BMPfileheader;

typedef struct tagBITMAPINFOHEADER
{
    DWORD biSize;
    LONG biWidth;
    LONG biHeight;
    WORD biPlanes;
    WORD biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG biXPelsPerMeter;
    LONG biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
    
    tagBITMAPINFOHEADER(){
        biSize = 40;
        biWidth = 1920;
        biHeight = 1080;
        biPlanes = 1;
        biBitCount = 24;
        biCompression = 0;
        biSizeImage = 6220800;
        biXPelsPerMeter = 2835;
        biYPelsPerMeter = 2835;
        biClrUsed = 0;
        biClrImportant = 0;
    }
}t_BMPinfoheader;
#pragma pack()

#endif /* bmp_h */
