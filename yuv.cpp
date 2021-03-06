//
//  yuv.cpp
//  SIMDLAB
//
//  Created by Nancy Fan on 12/26/16.
//  Copyright © 2016 Nancy Fan. All rights reserved.
//

#include "system.h"
#include "yuv.hpp"

void YUV::read(FILE * fp) const {
    char * buf = new char[size + (size >> 1)];
    fread(buf, sizeof(char), size + (size >> 1), fp);

    memcpy(pY8,                      buf, size * sizeof(char));
    memcpy(pU8,               buf + size, (size >> 2) * sizeof(char));
    memcpy(pV8, buf + size + (size >> 2), (size >> 2) * sizeof(char));
    
    delete[] buf;
}

void YUV::write(FILE * fp) const{
    char * buf = new char[size + (size >> 1)];
    memcpy(                     buf, pY8,  size * sizeof(char));
    memcpy(              buf + size, pU8,  (size >> 2) * sizeof(char));
    memcpy(buf + size + (size >> 2), pV8,  (size >> 2) * sizeof(char));
    
    fwrite(buf, sizeof(char), size + (size >> 1), fp);
    delete[] buf;
}

void YUV::u8_to_s16() const {
    for (int i = 0; i < size; i++) {
        this->pY16[i] = (int16_t)this->pY8[i];
    }
    for (int i = 0; i < (size >> 2); i++) {
        this->pU16[i] = (int16_t)this->pU8[i];
        this->pV16[i] = (int16_t)this->pV8[i];
    }
}

void YUV::s16_to_u8() const {
    for (int i = 0; i < size; i++) {
        this->pY8[i] = format(this->pY16[i]);
    }
    for (int i = 0; i < (size >> 2); i++) {
        this->pU8[i] = format(this->pU16[i]);
        this->pV8[i] = format(this->pV16[i]);
    }
}

void YUV::u8_to_s32() const {
    for (int i = 0; i < size; i++) {
        this->pY32[i] = (int32_t)this->pY8[i];
    }
    for (int i = 0; i < (size >> 2); i++) {
        this->pU32[i] = (int32_t)this->pU8[i];
        this->pV32[i] = (int32_t)this->pV8[i];
    }
}

void YUV::s32_to_u8() const {
    for (int i = 0; i < size; i++) {
        this->pY8[i] = format(this->pY32[i]);
    }
    for (int i = 0; i < (size >> 2); i++) {
        this->pU8[i] = format(this->pU32[i]);
        this->pV8[i] = format(this->pV32[i]);
    }
}
