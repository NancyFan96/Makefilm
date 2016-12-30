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

extern const char infile1[20];
extern const char infile2[20];
extern const char outfile01[20];
extern const char outfile02[20];
extern const char outfile11[20];
extern const char outfile12[20];
extern const char outfile21[20];
extern const char outfile22[20];
extern const char outfile31[20];
extern const char outfile32[20];


FILE *fin1, *fin2;
FILE *fout01, *fout02;
FILE *fout11, *fout12;
FILE *fout21, *fout22;
FILE *fout31, *fout32;
FILE *foutcheck1, *foutcheck2;

//#define RESERVE

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
    fclose(fin1);

    if((fin2 = fopen(infile2, "rb")) == NULL){
        cout << "INPUT FILE dem2.yuv ERROR." << endl;
        return -1;
    }
    DEM2_YUV.read(fin2);
    fclose(fin2);
    
#ifndef RESERVE
    if((fout01 = fopen(outfile01, "wb")) == NULL){
        cout << "OUTPUT FILE 01 ERROR." << endl;
        return -1;
    }
    if((fout02 = fopen(outfile02, "wb")) == NULL){
        cout << "OUTPUT FILE 02 ERROR." << endl;
        return -1;
    }
#endif
    if((fout11 = fopen(outfile11, "wb")) == NULL){
        cout << "OUTPUT FILE 11 ERROR." << endl;
        return -1;
    }
    if((fout12 = fopen(outfile12, "wb")) == NULL){
        cout << "OUTPUT FILE 12 ERROR." << endl;
        return -1;
    }

    if((fout21 = fopen(outfile21, "wb")) == NULL){
        cout << "OUTPUT FILE 21 ERROR." << endl;
        return -1;
    }
    if((fout22 = fopen(outfile22, "wb")) == NULL){
        cout << "OUTPUT FILE 22 ERROR." << endl;
        return -1;
    }

    if((fout31 = fopen(outfile31, "wb")) == NULL){
        cout << "OUTPUT FILE 31 ERROR." << endl;
        return -1;
    }
    if((fout32 = fopen(outfile32, "wb")) == NULL){
        cout << "OUTPUT FILE 32 ERROR." << endl;
        return -1;
    }

    
    if((foutcheck1 = fopen("output/check1.bmp", "wb")) == NULL){
        cout << "OUTPUT FILE checkfile ERROR." << endl;
        return -1;
    }
    if((foutcheck2 = fopen("output/check2.bmp", "wb")) == NULL){
        cout << "OUTPUT FILE checkfile ERROR." << endl;
        return -1;
    }

    
    cout << "Start Making Films..." << endl;
  
#ifndef RESERVE
    process_without_simd(OUT_YUV, DEM1_YUV, DEM2_YUV, CHECK_RGB, CHECK_RGB2, 0);
    process_without_simd(OUT_YUV, DEM1_YUV, DEM2_YUV, CHECK_RGB, CHECK_RGB2, 1);
    process_with_mmx(OUT_YUV, DEM1_YUV, DEM2_YUV, CHECK_RGB, CHECK_RGB2, 0);
    process_with_mmx(OUT_YUV, DEM1_YUV, DEM2_YUV, CHECK_RGB, CHECK_RGB2, 1);
#endif 
    process_with_sse(OUT_YUV, DEM1_YUV, DEM2_YUV, CHECK_RGB, CHECK_RGB2, 0);
    process_with_sse(OUT_YUV, DEM1_YUV, DEM2_YUV, CHECK_RGB, CHECK_RGB2, 1);
/*
    process_with_avx(OUT_YUV, DEM1_YUV, DEM2_YUV, CHECK_RGB, CHECK_RGB2, 0);
    process_with_avx(OUT_YUV, DEM1_YUV, DEM2_YUV, CHECK_RGB, CHECK_RGB2, 1);
*/
    cout << "\nFilms have made! Enjoy them!\nBye~\n" << endl;

    
    return 0;
}
