//
//  avx.cpp
//  SIMDLAB
//
//  Created by Nancy Fan on 12/26/16.
//  Copyright © 2016 Nancy Fan. All rights reserved.
//

#include "system.h"
#include "yuv.hpp"
#include "rgb.hpp"
// Header for AVX
#include <immintrin.h>

using namespace std;

extern FILE *fout31, *fout32;
extern FILE *foutcheck1, *foutcheck2;

extern void yuv2rgb_without_simd(const YUV & yuv, RGB & rgb);
extern void blending_without_simd(RGB & rgb_blending, const RGB & rgb1, const RGB & rgb2, const int A, const bool mode);
extern void rgb2yuv_without_simd(YUV & yuv,const RGB & rgb);

const float YUV2R_s[3] = {1.164383,         0,  1.596027};
const float YUV2G_s[3] = {1.164383, -0.391762, -0.812968};
const float YUV2B_s[3] = {1.164383,  2.017232,          0};

const float RGB2Y_s[3] = {0.256788, 0.504129, 0.097906};
const float RGB2U_s[3] = {-0.148223, -0.290993, 0.439216};
const float RGB2V_s[3] = {0.439216, -0.367788, -0.071427};

const __m256 Y2RGB_256s = _mm256_set1_ps(YUV2R_s[0]);
const __m256 V2R_256s = _mm256_set1_ps(YUV2R_s[2]);
const __m256 U2G_256s = _mm256_set1_ps(YUV2G_s[1]);
const __m256 V2G_256s = _mm256_set1_ps(YUV2G_s[2]);
const __m256 U2B_256s = _mm256_set1_ps(YUV2B_s[1]);

const __m256 R2Y_256s = _mm256_set1_ps(RGB2Y_s[0]);
const __m256 G2Y_256s = _mm256_set1_ps(RGB2Y_s[1]);
const __m256 B2Y_256s = _mm256_set1_ps(RGB2Y_s[2]);
const __m256 R2U_256s = _mm256_set1_ps(RGB2U_s[0]);
const __m256 G2U_256s = _mm256_set1_ps(RGB2U_s[1]);
const __m256 B2U_256s = _mm256_set1_ps(RGB2U_s[2]);
const __m256 R2V_256s = _mm256_set1_ps(RGB2V_s[0]);
const __m256 G2V_256s = _mm256_set1_ps(RGB2V_s[1]);
const __m256 B2V_256s = _mm256_set1_ps(RGB2V_s[2]);

const __m256 YConst16_256s = _mm256_set1_ps((float)16);
const __m256 UVConst128_256s = _mm256_set1_ps((float)128);

void yuv2rgb_with_avx(const YUV & yuv, RGB & rgb){
    
}
void blending_with_avx(RGB & rgb_blending, const RGB & rgb1, const RGB & rgb2, const int A, const bool mode){
    
}
void rgb2yuv_with_avx(YUV & yuv,const RGB & rgb){
    
}


int process_with_avx(YUV &OUT_YUV, const YUV &DEM1_YUV, const YUV &DEM2_YUV, RGB &CHECK_RGB1, RGB &CHECK_RGB2, const bool mode){
    clock_t begin_time = clock();
    clock_t total_time = 0;
    
    RGB rgb_blending = RGB(DEM1_YUV.width, DEM1_YUV.height);
    
    cout << "\nAVX..." << endl;
    clock_t core_time = clock();
    
    _mm_empty();
    yuv2rgb_with_avx(DEM1_YUV, CHECK_RGB1);
    if(mode)    yuv2rgb_with_avx(DEM2_YUV, CHECK_RGB2);
    
    for (int A = 1; A < 256; A += 3) {
        blending_with_avx(rgb_blending, CHECK_RGB1, CHECK_RGB2, A, mode);
        rgb2yuv_with_avx(OUT_YUV, rgb_blending);
        
        total_time += clock() - core_time;
        core_time = clock();
        
        if(mode)
            OUT_YUV.write(fout32);
        else
            OUT_YUV.write(fout31);
    }// process end
    if(mode)    fclose(fout32);
    else    fclose(fout31);
    
    if(mode){
        cout << "Alpha Blending with dem1.yuv and dem2.yuv, output file is \"alpha3-2.yuv\":" <<endl;
        cout << "Core function time: " << (double)total_time / CLOCKS_PER_SEC * 1000<< "ms" << endl;
        cout << "Include output time: " << (double)(clock() - begin_time) / CLOCKS_PER_SEC * 1000 << "ms" << endl << endl;
    }else{
        cout << "Alpha Blending with dem1.yuv, output file is \"alpha3-1.yuv\":" <<endl;
        cout << "Core function time: " << (double)total_time / CLOCKS_PER_SEC * 1000<< "ms" << endl;
        cout << "Include output time: " << (double)(clock() - begin_time) / CLOCKS_PER_SEC * 1000 << "ms" << endl << endl;
    }
    
    _mm_empty();
    return 0;
}
