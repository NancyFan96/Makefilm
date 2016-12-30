//
//  rgb.hpp
//  SIMDLAB
//
//  Created by Nancy Fan on 12/26/16.
//  Copyright © 2016 Nancy Fan. All rights reserved.
//

#ifndef rgb_hpp
#define rgb_hpp

#include <stdio.h>
#include <stdint.h>
#include <iostream>

class RGB{
public:
    int32_t width;
    int32_t height;
	int32_t size;
    
    uint8_t* pR8;
    uint8_t* pG8;
    uint8_t* pB8;
    
    int16_t* pR16;
    int16_t* pG16;
    int16_t* pB16;
    
    int32_t* pR32;
    int32_t* pG32;
    int32_t* pB32;

    
    RGB(int32_t _width, int32_t _height) {
        width = _width;
        height = _height;
        size = width * height;

        pR8 = new uint8_t[size];
        pG8 = new uint8_t[size];
        pB8 = new uint8_t[size];
        
        pR16 = new int16_t[size];
        pG16 = new int16_t[size];
        pB16 = new int16_t[size];
     
        pR32 = new int32_t[size];
        pG32 = new int32_t[size];
        pB32 = new int32_t[size];

        memset(pR8, 0, size * sizeof(uint8_t));
        memset(pG8, 0, size * sizeof(uint8_t));
        memset(pB8, 0, size * sizeof(uint8_t));
    }
    
    ~RGB() {
        delete[] pR8;
        delete[] pG8;
        delete[] pB8;
        delete[] pR16;
        delete[] pG16;
        delete[] pB16;
        delete[] pR32;
        delete[] pG32;
        delete[] pB32;
    }
    
    void write(FILE* fp) const;
    void u8_to_s16() const;
    void s16_to_u8() const;
    void u8_to_s32() const;
    void s32_to_u8() const;
    void update_32_16() const;
    void update_16_32() const;
    void round() const;

	/* Only for debug. Write 16bits data to a txt file. */
	void write_data_to_file(FILE* fp) const; 
		
};


#endif /* rgb_hpp */
