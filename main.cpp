//
//  main.cpp
//  SIMDLAB
//
//  Use MMX, SSE2, AVX to accelerate
//
//  Created by Nancy Fan on 12/16/16.
//  Copyright © 2016 Nancy Fan. All rights reserved.
//

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include "yuv.hpp"
#include "rgb.hpp"

using namespace std;

const int WIDTH = 1920;
const int HEIGHT = 1080;
const int SIZE = WIDTH * HEIGHT;
const int BLOCK_IN_WIDTH = WIDTH/2;


int* pR = new int[SIZE];
int* pG = new int[SIZE];
int* pB = new int[SIZE];

int mode = 0;
const char infile1[20] = "dem1.yuv";
const char infile2[20] = "dem2.yuv";
const char outfile1[20] = "alpha1.yuv";
const char outfile2[20] = "alpha2.yuv";


#define getBlockID(i) (i/(WIDTH*2)*(BLOCK_IN_WIDTH) + (i%WIDTH)/2)

void help()
{
    printf("Use dem1.yuv, dem1.yuv to process...\n");
    printf("Usage: ./makefilm <mode>\n");
    printf("       mode [default]: mode 1 and mode 2\n");
    printf("       mode 1: alpha blending with dem1.yuv\n");
    printf("       mode 2: alpha blending with dem1.yuv and dem2.yuv\n");
    return;
}


int main(int argc, char* argv[])
{
    if(argc > 2){
        help();
        return 0;
    }
    if(argc == 2)
        mode = atoi(argv[1]);
    if(!(mode == 0 || mode == 1 || mode ==2)){
        help();
        return 0;
    }
    
    FILE *fin1, *fin2, *fout1, *fout2;
    
    YUV DEM1_YUV = YUV(WIDTH, HEIGHT);
    YUV DEM2_YUV = YUV(WIDTH, HEIGHT);
    YUV OUT_YUV = YUV(WIDTH, HEIGHT);
    RGB CHECK_RGB = RGB(WIDTH, HEIGHT);
    RGB CHECK_RGB2 = RGB(WIDTH, HEIGHT);
    
    
    if((fin1 = fopen(infile1, "rb")) == NULL){
        cout << "INPUT FILE dem1.yuv ERROR." << endl;
        return -1;
    }
    DEM1_YUV.read(fin1);
    pclose(fin1);

    if((fin2 = fopen(infile2, "rb")) == NULL){
        cout << "INPUT FILE dem2.yuv ERROR." << endl;
        return -1;
    }
    DEM2_YUV.read(fin2);
    pclose(fin2);

    if((fout1 = fopen(outfile1, "wb")) == NULL){
        cout << "OUTPUT FILE 1 ERROR." << endl;
        return -1;
    }
    if((fout2 = fopen(outfile2, "wb")) == NULL){
        cout << "OUTPUT FILE 2 ERROR." << endl;
        return -1;
    }
    
    FILE *foutcheck;
    if((foutcheck = fopen("check.bmp", "wb")) == NULL){
        cout << "OUTPUT FILE checkfile ERROR." << endl;
        return -1;
    }
    
    cout << "start processing..." << endl;
    

    clock_t begin_time, total_time;
    begin_time = clock();
    total_time = 0;
    for(int A = 1; A < 256; A += 3){
        int iY = 0, iUV = 0, tmp;
        clock_t core_time = clock();
        for(int row = 0; row < DEM1_YUV.height; row++){
            for(int col = 0; col < DEM1_YUV.width; col++){
                int iiUV = getBlockID(iY);
                tmp = DEM1_YUV.pY16[iY] + (int)(1.140*(DEM1_YUV.pV16[iiUV] - 128));
                CHECK_RGB.pR16[iY] = tmp < 0 ? 0 : (tmp > 255 ? 255 : tmp);
                
                tmp = DEM1_YUV.pY16[iY] + (int)(-0.394*(DEM1_YUV.pU16[iiUV] - 128) - 0.518*(DEM1_YUV.pV16[iiUV] - 128));
                CHECK_RGB.pG16[iY] = tmp < 0 ? 0 : (tmp > 255 ? 255 : tmp);

                tmp = DEM1_YUV.pY16[iY] + (int)(2.032*(DEM1_YUV.pU16[iiUV] - 128));
                CHECK_RGB.pB16[iY] = tmp < 0 ? 0 : (tmp > 255 ? 255 : tmp);
        
                int RR = A * (CHECK_RGB.pR16[iY]) /256;
                int GG = A * (CHECK_RGB.pG16[iY]) /256;
                int BB = A * (CHECK_RGB.pB16[iY]) /256;
                
                OUT_YUV.pY16[iY] = 0.299*RR + 0.587*GG + 0.114*BB;
                
                if((row&1)&&(col&1)){
                    OUT_YUV.pU16[iUV] = -0.147*RR - 0.289*GG + 0.436*BB + 128;
                    OUT_YUV.pV16[iUV] = 0.615*RR - 0.515*GG - 0.100*BB + 128;
                    iUV++;
                }
                
                iY++;
            }
        }// get one picture
        
        total_time += clock() - core_time;
        //CHECK_RGB.write(foutcheck);
        OUT_YUV.write(fout1);
    }// process end
    
    fclose(fout1);
    
    cout << "Alpha Blending with dem1.yuv, output file is \"alpha1.yuv\":" <<endl;
    cout << "Core function time: " << (double)total_time / CLOCKS_PER_SEC * 1000<< "ms" << endl;
    cout << "Total run time: " << (double)(clock() - begin_time) / CLOCKS_PER_SEC * 1000 << "ms" << endl << endl;

    total_time = 0;
    for(int A = 1; A < 256; A += 3){
        int iY = 0, iUV = 0, tmp;
        clock_t core_time = clock();
        for(int row = 0; row < DEM1_YUV.height; row++){
            for(int col = 0; col < DEM1_YUV.width; col++){
                int iiUV = getBlockID(iY);
                tmp = DEM1_YUV.pY16[iY] + (int)(1.140*(DEM1_YUV.pV16[iiUV] - 128));
                CHECK_RGB.pR16[iY] = tmp < 0 ? 0 : (tmp > 255 ? 255 : tmp);
                
                tmp = DEM1_YUV.pY16[iY] + (int)(-0.394*(DEM1_YUV.pU16[iiUV] - 128) - 0.518*(DEM1_YUV.pV16[iiUV] - 128));
                CHECK_RGB.pG16[iY] = tmp < 0 ? 0 : (tmp > 255 ? 255 : tmp);
                
                tmp = DEM1_YUV.pY16[iY] + (int)(2.032*(DEM1_YUV.pU16[iiUV] - 128));
                CHECK_RGB.pB16[iY] = tmp < 0 ? 0 : (tmp > 255 ? 255 : tmp);
                
                tmp = DEM2_YUV.pY16[iY] + (int)(1.140*(DEM2_YUV.pV16[iiUV] - 128));
                CHECK_RGB2.pR16[iY] = tmp < 0 ? 0 : (tmp > 255 ? 255 : tmp);
                
                tmp = DEM2_YUV.pY16[iY] + (int)(-0.394*(DEM2_YUV.pU16[iiUV] - 128) - 0.518*(DEM2_YUV.pV16[iiUV] - 128));
                CHECK_RGB2.pG16[iY] = tmp < 0 ? 0 : (tmp > 255 ? 255 : tmp);
                
                tmp = DEM2_YUV.pY16[iY] + (int)(2.032*(DEM2_YUV.pU16[iiUV] - 128));
                CHECK_RGB2.pB16[iY] = tmp < 0 ? 0 : (tmp > 255 ? 255 : tmp);

                int _A = 256 - A;
                int RR = (A * CHECK_RGB.pR16[iY] + _A * CHECK_RGB2.pR16[iY]) /256;
                int GG = (A * CHECK_RGB.pG16[iY] + _A * CHECK_RGB2.pG16[iY]) /256;
                int BB = (A * CHECK_RGB.pB16[iY] + _A * CHECK_RGB2.pB16[iY]) /256;
                
                OUT_YUV.pY16[iY] = 0.299*RR + 0.587*GG + 0.114*BB;
                
                if((row&1)&&(col&1)){
                    OUT_YUV.pU16[iUV] = -0.147*RR - 0.289*GG + 0.436*BB + 128;
                    OUT_YUV.pV16[iUV] = 0.615*RR - 0.515*GG - 0.100*BB + 128;
                    iUV++;
                }
                
                iY++;
            }
        }// get one picture
        
        total_time += clock() - core_time;
        //CHECK_RGB.write(foutcheck);
        OUT_YUV.write(fout2);
    }// process end
    
    fclose(fout2);
    
    cout << "Alpha Blending with dem1.yuv and dem2.yuv, output file is \"alpha2.yuv\":" <<endl;
    cout << "Core function time: " << (double)total_time / CLOCKS_PER_SEC * 1000<< "ms" << endl;
    cout << "Total run time: " << (double)(clock() - begin_time) / CLOCKS_PER_SEC * 1000 << "ms" << endl << endl;

    
    
    return 0;
}
