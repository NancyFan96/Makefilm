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
extern FILE *foutcheck;

int process_without_simd(YUV &OUT_YUV, YUV &DEM1_YUV, YUV &DEM2_YUV, RGB &CHECK_RGB1, RGB &CHECK_RGB2){
    clock_t begin_time, total_time;
    begin_time = clock();
    total_time = 0;
    
    cout << "\nNO SIMD..." << endl;

    /*************** YUV2RGB ****************/

    int tmp;
    clock_t core_time = clock();
    for(int iY = 0; iY < DEM1_YUV.size; iY++){
        int iUV = DEM1_YUV.getBlockID(iY);
        tmp = DEM1_YUV.pY16[iY] + (int)(1.140*(DEM1_YUV.pV16[iUV] - 128));
        CHECK_RGB1.pR16[iY] = tmp < 0 ? 0 : (tmp > 255 ? 255 : tmp);
        
        tmp = DEM1_YUV.pY16[iY] + (int)(-0.394*(DEM1_YUV.pU16[iUV] - 128) - 0.518*(DEM1_YUV.pV16[iUV] - 128));
        CHECK_RGB1.pG16[iY] = tmp < 0 ? 0 : (tmp > 255 ? 255 : tmp);
        
        tmp = DEM1_YUV.pY16[iY] + (int)(2.032*(DEM1_YUV.pU16[iUV] - 128));
        CHECK_RGB1.pB16[iY] = tmp < 0 ? 0 : (tmp > 255 ? 255 : tmp);
    }
    
    /*************** Blending ****************/
    for(int A = 1; A < 256; A += 3){
        
        /*************** RGB2YUV ****************/
        for(int iY = 0; iY < DEM1_YUV.size; iY++){
            int RR = A * (CHECK_RGB1.pR16[iY]) /256;
            int GG = A * (CHECK_RGB1.pG16[iY]) /256;
            int BB = A * (CHECK_RGB1.pB16[iY]) /256;
            
            OUT_YUV.pY16[iY] = 0.299*RR + 0.587*GG + 0.114*BB;
            
            int iUV = DEM1_YUV.getBlockID(iY);
            if(((iY/DEM1_YUV.width)&1)&&((iY%DEM1_YUV.width)&1)){
                OUT_YUV.pU16[iUV] = -0.147*RR - 0.289*GG + 0.436*BB + 128;
                OUT_YUV.pV16[iUV] = 0.615*RR - 0.515*GG - 0.100*BB + 128;
            }
        }// get one picture

        
        total_time += clock() - core_time;
        core_time = clock();
        if(A == 253)
        CHECK_RGB1.write(foutcheck);
        OUT_YUV.write(fout01);
    }// process end
    
    fclose(fout01);
    
    cout << "Alpha Blending with dem1.yuv, output file is \"alpha0-1.yuv\":" <<endl;
    cout << "Core function time: " << (double)total_time / CLOCKS_PER_SEC * 1000<< "ms" << endl;
    cout << "Total run time: " << (double)(clock() - begin_time) / CLOCKS_PER_SEC * 1000 << "ms" << endl << endl;
    
    total_time = 0;
    core_time = clock();
    for(int iY = 0; iY < DEM1_YUV.size; iY++){
        int iUV = DEM1_YUV.getBlockID(iY);
        tmp = DEM1_YUV.pY16[iY] + (int)(1.140*(DEM1_YUV.pV16[iUV] - 128));
        CHECK_RGB1.pR16[iY] = tmp < 0 ? 0 : (tmp > 255 ? 255 : tmp);
        
        tmp = DEM1_YUV.pY16[iY] + (int)(-0.394*(DEM1_YUV.pU16[iUV] - 128) - 0.518*(DEM1_YUV.pV16[iUV] - 128));
        CHECK_RGB1.pG16[iY] = tmp < 0 ? 0 : (tmp > 255 ? 255 : tmp);
        
        tmp = DEM1_YUV.pY16[iY] + (int)(2.032*(DEM1_YUV.pU16[iUV] - 128));
        CHECK_RGB1.pB16[iY] = tmp < 0 ? 0 : (tmp > 255 ? 255 : tmp);
    }
    for(int iY = 0; iY < DEM2_YUV.size; iY++){
        int iUV = DEM2_YUV.getBlockID(iY);
        tmp = DEM2_YUV.pY16[iY] + (int)(1.140*(DEM2_YUV.pV16[iUV] - 128));
        CHECK_RGB2.pR16[iY] = tmp < 0 ? 0 : (tmp > 255 ? 255 : tmp);
        
        tmp = DEM2_YUV.pY16[iY] + (int)(-0.394*(DEM2_YUV.pU16[iUV] - 128) - 0.518*(DEM2_YUV.pV16[iUV] - 128));
        CHECK_RGB2.pG16[iY] = tmp < 0 ? 0 : (tmp > 255 ? 255 : tmp);
        
        tmp = DEM2_YUV.pY16[iY] + (int)(2.032*(DEM2_YUV.pU16[iUV] - 128));
        CHECK_RGB2.pB16[iY] = tmp < 0 ? 0 : (tmp > 255 ? 255 : tmp);
    }

    
    /*************** Blending ****************/
    for(int A = 1; A < 256; A += 3){

        /*************** RGB2YUV ****************/
        for(int iY = 0; iY < DEM1_YUV.size; iY++){
                int _A = 256 - A;
                int RR = (A * CHECK_RGB1.pR16[iY] + _A * CHECK_RGB2.pR16[iY]) /256;
                int GG = (A * CHECK_RGB1.pG16[iY] + _A * CHECK_RGB2.pG16[iY]) /256;
                int BB = (A * CHECK_RGB1.pB16[iY] + _A * CHECK_RGB2.pB16[iY]) /256;
                
                OUT_YUV.pY16[iY] = 0.299*RR + 0.587*GG + 0.114*BB;
                int iUV = DEM1_YUV.getBlockID(iY);
                if(((iY/DEM1_YUV.width)&1)&&((iY%DEM1_YUV.width)&1)){
                    OUT_YUV.pU16[iUV] = -0.147*RR - 0.289*GG + 0.436*BB + 128;
                    OUT_YUV.pV16[iUV] = 0.615*RR - 0.515*GG - 0.100*BB + 128;
                }
                
        }// get one picture
        
        total_time += clock() - core_time;
        core_time = clock();
        //CHECK_RGB1.write(foutcheck);
        OUT_YUV.write(fout02);
    }// process end
    
    fclose(fout02);
    
    cout << "Alpha Blending with dem1.yuv and dem2.yuv, output file is \"alpha0-2.yuv\":" <<endl;
    cout << "Core function time: " << (double)total_time / CLOCKS_PER_SEC * 1000<< "ms" << endl;
    cout << "Total run time: " << (double)(clock() - begin_time) / CLOCKS_PER_SEC * 1000 << "ms" << endl << endl;
    
    return 0;
}
