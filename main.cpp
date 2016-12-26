//
//  main.cpp
//  SIMDLAB
//
//  Use MMX, SSE2, AVX to accelerate
//
//  Created by Nancy Fan on 12/16/16.
//  Copyright © 2016 Nancy Fan. All rights reserved.
//
#include "system.h"

using namespace std;

extern const int WIDTH;
extern const int HEIGHT;
extern const int SIZE;

//extern int mode;
extern const char infile1[20];
extern const char infile2[20];
extern const char outfile1[20];
extern const char outfile2[20];

FILE *fin1, *fin2, *fout1, *fout2;
FILE *foutcheck;

void help()
{
    printf("Use dem1.yuv, dem1.yuv to process...\n");
    printf("Usage: ./makefilm <mode>\n");
    printf("       mode [default]: mode 1 and mode 2\n");
    //printf("       mode 1: alpha blending with dem1.yuv\n");
    //printf("       mode 2: alpha blending with dem1.yuv and dem2.yuv\n");
    return;
}


int main(int argc, char* argv[])
{
    if(argc > 2){
        help();
        return 0;
    }
    
    /*if(argc == 2)
        mode = atoi(argv[1]);
    if(!(mode == 0 || mode == 1 || mode ==2)){
        help();
        return 0;
    }*/
    
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
    
    if((foutcheck = fopen("check.bmp", "wb")) == NULL){
        cout << "OUTPUT FILE checkfile ERROR." << endl;
        return -1;
    }
    
    cout << "start processing..." << endl;
    
    process_without_simd(OUT_YUV, DEM1_YUV, DEM2_YUV, CHECK_RGB, CHECK_RGB2);
    process_with_mmx(OUT_YUV, DEM1_YUV, DEM2_YUV, CHECK_RGB, CHECK_RGB2);
    process_with_sse(OUT_YUV, DEM1_YUV, DEM2_YUV, CHECK_RGB, CHECK_RGB2);
    process_with_avx(OUT_YUV, DEM1_YUV, DEM2_YUV, CHECK_RGB, CHECK_RGB2);

    
    return 0;
}
