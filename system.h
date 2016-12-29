//
//  system.h
//  SIMDLAB
//
//  Created by Nancy Fan on 12/26/16.
//  Copyright © 2016 Nancy Fan. All rights reserved.
//

#ifndef system_h
#define system_h

#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include "yuv.hpp"
#include "rgb.hpp"

const int WIDTH = 1920;
const int HEIGHT = 1080;
const int SIZE = WIDTH * HEIGHT;
const int BLOCK_IN_WIDTH = WIDTH/2;

//int mode = 0;
const char infile1[20] = "input/dem1.yuv";
const char infile2[20] = "input/dem2.yuv";
const char outfile01[20] = "output/alpha0-1.yuv";
const char outfile02[20] = "output/alpha0-2.yuv";
const char outfile11[20] = "output/alpha1-1.yuv";
const char outfile12[20] = "output/alpha1-2.yuv";
const char outfile21[20] = "output/alpha2-1.yuv";
const char outfile22[20] = "output/alpha2-2.yuv";
const char outfile31[20] = "output/alpha3-1.yuv";
const char outfile32[20] = "output/alpha3-2.yuv";


int process_without_simd(YUV &OUT_YUV, YUV &DEM1_YUV, YUV &DEM2_YUV, RGB &CHECK_RGB, RGB &CHECK_RGB2, bool mode);
int process_with_mmx(YUV &OUT_YUV, YUV &DEM1_YUV, YUV &DEM2_YUV, RGB &CHECK_RGB, RGB &CHECK_RGB2, bool mode);
int process_with_sse(YUV &OUT_YUV, YUV &DEM1_YUV, YUV &DEM2_YUV, RGB &CHECK_RGB, RGB &CHECK_RGB2, bool mode);
int process_with_avx(YUV &OUT_YUV, YUV &DEM1_YUV, YUV &DEM2_YUV, RGB &CHECK_RGB, RGB &CHECK_RGB2, bool mode);

inline uint8_t format(int16_t input) {
    if (input > 255) {
        return (uint8_t)255;
    }
    if (input < 0) {
        return (uint8_t)0;
    }
    return (uint8_t)input;
}

#endif /* system_h */
