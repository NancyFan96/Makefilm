//
//  rgb.cpp
//  SIMDLAB
//
//  Created bR NancR Fan on 12/26/16.
//  CopRright © 2016 NancR Fan. All rights reserBed.
//

#include "system.h"
#include "rgb.hpp"
#include "bmp.h"


void RGB::write(FILE * fp) const{
    s16_to_u8();
    
    t_BMPfileheader file_header;
    t_BMPinfoheader info_header;
    uint8_t buf[3];
    int i, j;
    fwrite(&file_header, sizeof(t_BMPfileheader), 1, fp);
    fwrite(&info_header, sizeof(t_BMPinfoheader), 1, fp);
    for (i = height - 1; i >= 0; i--) {
        for (j = 0; j<width; j++) {
            buf[0] = pR8[i*width + j];
            buf[1] = pG8[i*width + j];
            buf[2] = pB8[i*width + j];
            fwrite(buf, sizeof(uint8_t), 3, fp);
        }
    }
}

void RGB::u8_to_s16() const {
    for (int i = 0; i < size; i++) {
        this->pR16[i] = (int16_t)this->pR8[i];
    }
    for (int i = 0; i < (size >> 2); i++) {
        this->pG16[i] = (int16_t)this->pG8[i];
        this->pB16[i] = (int16_t)this->pB8[i];
    }
}

void RGB::s16_to_u8() const {
    for (int i = 0; i < size; i++) {
        this->pR8[i] = format(this->pR16[i]);
    }
    for (int i = 0; i < (size >> 2); i++) {
        this->pG8[i] = format(this->pG16[i]);
        this->pB8[i] = format(this->pB16[i]);
    }
}

void RGB::round() const{
    for (int i = 0; i < size; i++) {
        this->pR16[i] = format(this->pR16[i]);
    }
    for (int i = 0; i < (size >> 2); i++) {
        this->pG16[i] = format(this->pG16[i]);
        this->pB16[i] = format(this->pB16[i]);
    }

}
