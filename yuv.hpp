//
//  yuv.hpp
//  SIMDLAB
//
//  Created by Nancy Fan on 12/26/16.
//  Copyright © 2016 Nancy Fan. All rights reserved.
//

#ifndef yuv_hpp
#define yuv_hpp

#include <stdio.h>
#include <stdint.h>
#include <iostream>

class YUV {
public:
    int32_t width;
    int32_t height;
    int64_t size;
    
    uint8_t* pY8;
    uint8_t* pU8;
    uint8_t* pV8;
    
    int16_t* pY16;
    int16_t* pU16;
    int16_t* pV16;
    
    int32_t* pY32;
    int32_t* pU32;
    int32_t* pV32;

    
    YUV(int32_t _width, int32_t _height) {
        width = _width;
        height = _height;
        size = width * height;
        
        pY8 = new uint8_t[size];
        pU8 = new uint8_t[size >> 2];
        pV8 = new uint8_t[size >> 2];
        
        pY16 = new int16_t[size];
        pU16 = new int16_t[size >> 2];
        pV16 = new int16_t[size >> 2];
        
        pY32 = new int32_t[size];
        pU32 = new int32_t[size >> 2];
        pV32 = new int32_t[size >> 2];

       
        memset(pY8, 0, size * sizeof(int8_t));
        memset(pU8, 0, (size >> 2) * sizeof(int8_t));
        memset(pV8, 0, (size >> 2) * sizeof(int8_t));
    }
    
    ~YUV() {
        delete[] pY8;
        delete[] pU8;
        delete[] pV8;
        delete[] pY16;
        delete[] pU16;
        delete[] pV16;
    }
    
    void read(FILE* fp) const;
    void write(FILE* fp) const;
    void u8_to_s16() const;
    void s16_to_u8() const;
    void u8_to_s32() const;
    void s32_to_u8() const;

    int getBlockID(int iY) const { return (iY/(width*2)*(width/2) + (iY%width)/2);}
    
};


#endif /* yuv_hpp */
