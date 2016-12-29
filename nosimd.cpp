//
//  nosimd.cpp
//  SIMDLAB
//
//  Created by Nancy Fan on 12/26/16.
//  Copyright © 2016 Nancy Fan. All rights reserved.
//
#include "system.h"
#include "yuv.hpp"
#include "rgb.hpp"

using namespace std;

extern FILE *fout01, *fout02;
extern FILE *foutcheck1, *foutcheck2;

void yuv2rgb_without_simd(YUV & yuv, RGB & rgb){
    int tmp;
    for(int iY = 0; iY < yuv.size; iY++){
        int iUV = yuv.getBlockID(iY);
        tmp = yuv.pY16[iY] + (int)(1.140*(yuv.pV16[iUV] - 128));
        rgb.pR16[iY] = tmp < 0 ? 0 : (tmp > 255 ? 255 : tmp);
        
        tmp = yuv.pY16[iY] + (int)(-0.394*(yuv.pU16[iUV] - 128) - 0.518*(yuv.pV16[iUV] - 128));
        rgb.pG16[iY] = tmp < 0 ? 0 : (tmp > 255 ? 255 : tmp);
        
        tmp = yuv.pY16[iY] + (int)(2.032*(yuv.pU16[iUV] - 128));
        rgb.pB16[iY] = tmp < 0 ? 0 : (tmp > 255 ? 255 : tmp);
    }
}

void blending_without_simd(RGB & rgb1, RGB & rgb2, int A, bool mode){
    if(mode){
        int _A = 256 - A;
        for(int iY = 0; iY < rgb1.size; iY++){
            rgb1.pR16[iY] = (A * rgb1.pR16[iY] + _A * rgb2.pR16[iY]) >> 8;
            rgb1.pG16[iY] = (A * rgb1.pG16[iY] + _A * rgb2.pG16[iY]) >> 8;
            rgb1.pB16[iY] = (A * rgb1.pB16[iY] + _A * rgb2.pB16[iY]) >> 8;
        }
    }
    else{
        for(int iY = 0; iY < rgb1.size; iY++){
            rgb1.pR16[iY] = ((uint16_t)A * rgb1.pR16[iY]) >> 8;
            rgb1.pG16[iY] = ((uint16_t)A * rgb1.pG16[iY]) >> 8;
            rgb1.pB16[iY] = ((uint16_t)A * rgb1.pB16[iY]) >> 8;
        }
    }
}

void rgb2yuv_without_simd(YUV & yuv, RGB & rgb){
    for(int row = 0, iY = 0, iUV = 0; row < yuv.height; row++){
        for(int col = 0; col < yuv.width; col++, iY++){
            yuv.pY16[iY] = 0.299*rgb.pR16[iY] + 0.587*rgb.pG16[iY] + 0.114*rgb.pB16[iY];
            
            if((row&1)&&(col&1)){
                yuv.pU16[iUV] = -0.147*rgb.pR16[iY] - 0.289*rgb.pG16[iY] + 0.436*rgb.pB16[iY] + 128;
                yuv.pV16[iUV] = 0.615*rgb.pR16[iY] - 0.515*rgb.pG16[iY] - 0.100*rgb.pB16[iY] + 128;
                iUV++;
            }
        }
    }// get one picture

}

int process_without_simd(YUV &OUT_YUV, YUV &DEM1_YUV, YUV &DEM2_YUV, RGB &CHECK_RGB1, RGB &CHECK_RGB2, bool mode){
    clock_t begin_time = clock();
    clock_t total_time = 0;
    
    cout << "\nNO SIMD..." << endl;
    clock_t core_time = clock();

    yuv2rgb_without_simd(DEM1_YUV, CHECK_RGB1);
    if(mode)    yuv2rgb_without_simd(DEM2_YUV, CHECK_RGB2);

    CHECK_RGB1.write(foutcheck1);
    
    /*************** Blending ****************/
    for(int A = 1; A < 256; A += 3){
        blending_without_simd(CHECK_RGB1, CHECK_RGB2, A, mode);
        if(A == 253)
            CHECK_RGB1.write(foutcheck2);
        rgb2yuv_without_simd(OUT_YUV, CHECK_RGB1);
        
        total_time += clock() - core_time;
        core_time = clock();
        
        if(mode)
            OUT_YUV.write(fout02);
        else
            OUT_YUV.write(fout01);
    }// process end
    if(mode)    fclose(fout02);
    else    fclose(fout01);
    
    if(mode){
        cout << "Alpha Blending with dem1.yuv and dem2.yuv, output file is \"alpha0-2.yuv\":" <<endl;
        cout << "Core function time: " << (double)total_time / CLOCKS_PER_SEC * 1000<< "ms" << endl;
        cout << "Include output time: " << (double)(clock() - begin_time) / CLOCKS_PER_SEC * 1000 << "ms" << endl << endl;
    }else{
        cout << "Alpha Blending with dem1.yuv, output file is \"alpha0-1.yuv\":" <<endl;
        cout << "Core function time: " << (double)total_time / CLOCKS_PER_SEC * 1000<< "ms" << endl;
        cout << "Include output time: " << (double)(clock() - begin_time) / CLOCKS_PER_SEC * 1000 << "ms" << endl << endl;
    }
    
    
    return 0;
}
