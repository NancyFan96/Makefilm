//
//  mmx.cpp
//  SIMDLAB
//
//  Created by Nancy Fan on 12/26/16.
//  Copyright © 2016 Nancy Fan. All rights reserved.
//

#include "system.h"
#include "yuv.hpp"
#include "rgb.hpp"
// Header for MMX
#include <mmintrin.h>
#include <emmintrin.h>


using namespace std;

extern FILE *fout11, *fout12;
extern FILE *foutcheck;

int process_with_mmx(YUV &OUT_YUV, YUV &DEM1_YUV, YUV &DEM2_YUV, RGB &CHECK_RGB, RGB &CHECK_RGB2){
    clock_t begin_time, total_time;
    begin_time = clock();
    total_time = 0;
    
    cout << "\nMMX..." << endl;

    int nloop = DEM1_YUV.size >> 2;
    __m64 * Yp16_1 = (__m64 *)DEM1_YUV.pY16;
    __m64 * Up16_1 = (__m64 *)DEM1_YUV.pU16;
    __m64 * Vp16_1 = (__m64 *)DEM1_YUV.pV16;
    const __m64 sub128 = _mm_set_pi16(128, 128, 128, 128);
    
    __m64 tmp;
    
    _mm_empty();
    
    
    for (int A = 1; A < 256; A += 3) {
        clock_t core_time = clock();
        for(int i = 0; i < nloop; i++){
            
        }
    }
    
    
    
    
    
    
    for(int A = 1; A < 256; A += 3){
        int iY = 0, tmp;
        clock_t core_time = clock();
        for(int iY = 0; iY < DEM1_YUV.size; iY++){
                int iUV = DEM1_YUV.getBlockID(iY);
                tmp = DEM1_YUV.pY16[iY] + (int)(1.140*(DEM1_YUV.pV16[iUV] - 128));
                CHECK_RGB.pR16[iY] = tmp < 0 ? 0 : (tmp > 255 ? 255 : tmp);
                
                tmp = DEM1_YUV.pY16[iY] + (int)(-0.394*(DEM1_YUV.pU16[iUV] - 128) - 0.518*(DEM1_YUV.pV16[iUV] - 128));
                CHECK_RGB.pG16[iY] = tmp < 0 ? 0 : (tmp > 255 ? 255 : tmp);
                
                tmp = DEM1_YUV.pY16[iY] + (int)(2.032*(DEM1_YUV.pU16[iUV] - 128));
                CHECK_RGB.pB16[iY] = tmp < 0 ? 0 : (tmp > 255 ? 255 : tmp);
                
                int RR = A * (CHECK_RGB.pR16[iY]) /256;
                int GG = A * (CHECK_RGB.pG16[iY]) /256;
                int BB = A * (CHECK_RGB.pB16[iY]) /256;
                
                OUT_YUV.pY16[iY] = 0.299*RR + 0.587*GG + 0.114*BB;
                
                if(((iY/DEM1_YUV.width)&1)&&((iY%DEM1_YUV.width)&1)){
                    OUT_YUV.pU16[iUV] = -0.147*RR - 0.289*GG + 0.436*BB + 128;
                    OUT_YUV.pV16[iUV] = 0.615*RR - 0.515*GG - 0.100*BB + 128;
                }
        }// get one picture
        
        total_time += clock() - core_time;
        if(A == 253)
            CHECK_RGB.write(foutcheck);
        OUT_YUV.write(fout11);
    }// process end
    
    fclose(fout11);
    fclose(foutcheck);
    
    cout << "Alpha Blending with dem1.yuv, output file is \"alpha0-1.yuv\":" <<endl;
    cout << "Core function time: " << (double)total_time / CLOCKS_PER_SEC * 1000<< "ms" << endl;
    cout << "Total run time: " << (double)(clock() - begin_time) / CLOCKS_PER_SEC * 1000 << "ms" << endl << endl;
    
    total_time = 0;
    for(int A = 1; A < 256; A += 3){
        int iY = 0, iUV = 0, tmp;
        clock_t core_time = clock();
        for(int row = 0; row < DEM1_YUV.height; row++){
            for(int col = 0; col < DEM1_YUV.width; col++){
                int iUV = DEM1_YUV.getBlockID(iY);
                tmp = DEM1_YUV.pY16[iY] + (int)(1.140*(DEM1_YUV.pV16[iUV] - 128));
                CHECK_RGB.pR16[iY] = tmp < 0 ? 0 : (tmp > 255 ? 255 : tmp);
                
                tmp = DEM1_YUV.pY16[iY] + (int)(-0.394*(DEM1_YUV.pU16[iUV] - 128) - 0.518*(DEM1_YUV.pV16[iUV] - 128));
                CHECK_RGB.pG16[iY] = tmp < 0 ? 0 : (tmp > 255 ? 255 : tmp);
                
                tmp = DEM1_YUV.pY16[iY] + (int)(2.032*(DEM1_YUV.pU16[iUV] - 128));
                CHECK_RGB.pB16[iY] = tmp < 0 ? 0 : (tmp > 255 ? 255 : tmp);
                
                tmp = DEM2_YUV.pY16[iY] + (int)(1.140*(DEM2_YUV.pV16[iUV] - 128));
                CHECK_RGB2.pR16[iY] = tmp < 0 ? 0 : (tmp > 255 ? 255 : tmp);
                
                tmp = DEM2_YUV.pY16[iY] + (int)(-0.394*(DEM2_YUV.pU16[iUV] - 128) - 0.518*(DEM2_YUV.pV16[iUV] - 128));
                CHECK_RGB2.pG16[iY] = tmp < 0 ? 0 : (tmp > 255 ? 255 : tmp);
                
                tmp = DEM2_YUV.pY16[iY] + (int)(2.032*(DEM2_YUV.pU16[iUV] - 128));
                CHECK_RGB2.pB16[iY] = tmp < 0 ? 0 : (tmp > 255 ? 255 : tmp);
                
                int _A = 256 - A;
                int RR = (A * CHECK_RGB.pR16[iY] + _A * CHECK_RGB2.pR16[iY]) /256;
                int GG = (A * CHECK_RGB.pG16[iY] + _A * CHECK_RGB2.pG16[iY]) /256;
                int BB = (A * CHECK_RGB.pB16[iY] + _A * CHECK_RGB2.pB16[iY]) /256;
                
                OUT_YUV.pY16[iY] = 0.299*RR + 0.587*GG + 0.114*BB;
                
                if((row&1)&&(col&1)){
                    OUT_YUV.pU16[iUV] = -0.147*RR - 0.289*GG + 0.436*BB + 128;
                    OUT_YUV.pV16[iUV] = 0.615*RR - 0.515*GG - 0.100*BB + 128;
                }
                
                iY++;
            }
        }// get one picture
        
        total_time += clock() - core_time;
        //CHECK_RGB.write(foutcheck);
        OUT_YUV.write(fout12);
    }// process end
    
    fclose(fout12);
    
    cout << "Alpha Blending with dem1.yuv and dem2.yuv, output file is \"alpha0-2.yuv\":" <<endl;
    cout << "Core function time: " << (double)total_time / CLOCKS_PER_SEC * 1000<< "ms" << endl;
    cout << "Total run time: " << (double)(clock() - begin_time) / CLOCKS_PER_SEC * 1000 << "ms" << endl << endl;

    
    
    return 0;
}
