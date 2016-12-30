//
//  sse.cpp
//  SIMDLAB
//
//  Created by Nancy Fan on 12/26/16.
//  Copyright © 2016 Nancy Fan. All rights reserved.
//

#include "system.h"
#include "yuv.hpp"
#include "rgb.hpp"
// Header for SSE2
#include <emmintrin.h>

using namespace std;

extern FILE *fout21, *fout22;
extern FILE *foutcheck1, *foutcheck2;

extern void yuv2rgb_without_simd(const YUV & yuv, RGB & rgb);
extern void blending_without_simd(RGB & rgb_blending, const RGB & rgb1, const RGB & rgb2, const int A, const bool mode);
extern void rgb2yuv_without_simd(YUV & yuv,const RGB & rgb);

const float YUV2R_s[3] = {0.164383,         0,  0.596027};
const float YUV2G_s[3] = {0.164383, -0.391762, -0.812968};
const float YUV2B_s[3] = {0.164383,  0.017232,          0};

const float RGB2Y_s[3] = {0.256788, 0.504129, 0.097906};
const float RGB2U_s[3] = {-0.148223, -0.290993, 0.439216};
const float RGB2V_s[3] = {0.439216, -0.367788, -0.071427};

const __m128 Y2RGB_s = _mm_set1_ps(YUV2R_s[0]);
const __m128 V2R_s = _mm_set1_ps(YUV2R_s[2]);
const __m128 U2G_s = _mm_set1_ps(YUV2G_s[1]);
const __m128 V2G_s = _mm_set1_ps(YUV2G_s[2]);
const __m128 U2B_s = _mm_set1_ps(YUV2B_s[1]);

const __m128 R2Y_s = _mm_set1_ps(RGB2Y_s[0]);
const __m128 G2Y_s = _mm_set1_ps(RGB2Y_s[1]);
const __m128 B2Y_s = _mm_set1_ps(RGB2Y_s[2]);
const __m128 R2U_s = _mm_set1_ps(RGB2U_s[0]);
const __m128 G2U_s = _mm_set1_ps(RGB2U_s[1]);
const __m128 B2U_s = _mm_set1_ps(RGB2U_s[2]);
const __m128 R2V_s = _mm_set1_ps(RGB2V_s[0]);
const __m128 G2V_s = _mm_set1_ps(RGB2V_s[1]);
const __m128 B2V_s = _mm_set1_ps(RGB2V_s[2]);

const __m128 YConst16_s = _mm_set1_ps((float)16);
const __m128 UVConst128_s = _mm_set1_ps((float)128);

/*
 * yuv2rgb
 * R = 1.164383 * (Y - 16) + 1.596027*(V - 128)
 * G = 1.164383 * (Y - 16) – 0.391762*(U - 128) – 0.812968*(V - 128)
 * B = 1.164383 * (Y - 16) + 2.017232*(U - 128)
 */
void yuv2rgb_with_sse(const YUV & yuv, RGB & rgb){
    yuv.u8_to_s32();
    // duplicate U and V, preprocess for pack computing
    int32_t *dupPU32 = new int32_t[yuv.size];
    int32_t *dupPV32 = new int32_t[yuv.size];
    for(int i = 0; i < yuv.size; i++){
        int iUV = yuv.getBlockID(i);
        dupPU32[i] = yuv.pU32[iUV];
        dupPV32[i] = yuv.pV32[iUV];
    }
    _mm_empty();
    
    __m128 tmp, tmpY, tmpU, tmpV;
    __m128 * dstR = (__m128*) rgb.pR32;
    __m128 * dstG = (__m128*) rgb.pG32;
    __m128 * dstB = (__m128*) rgb.pB32;
    
    __m128 * Yp32 = (__m128 *)yuv.pY32;
    __m128 * Up32 = (__m128 *)dupPU32;
    __m128 * Vp32 = (__m128 *)dupPV32;
    
    int64_t nloop = yuv.size >> 2;
    for(int i = 0; i < nloop; i++){
        Yp32[i] = _mm_cvtepi32_ps(Yp32[i]);
        Up32[i] = _mm_cvtepi32_ps(Up32[i]);
        Vp32[i] = _mm_cvtepi32_ps(Vp32[i]);
        
        dstR[i] = _mm_setzero_ps();
        dstG[i] = _mm_setzero_ps();
        dstB[i] = _mm_setzero_ps();
        
        tmpY = _mm_sub_ps(Yp32[i], YConst16_s);
        tmpY = _mm_mul_ps(tmpY, Y2RGB_s);           // tmpY = 1.164383 * (Y - 16)
        tmpU = _mm_sub_ps(Up32[i], UVConst128_s);   // tmpU = (U - 128)
        tmpV = _mm_sub_ps(Vp32[i], UVConst128_s);   // tmpV = (V - 128)
        
        dstR[i] = _mm_add_ps(dstR[i], tmpY);
        tmp = _mm_mul_ps(tmpV, V2R_s);
        dstR[i] = _mm_add_ps(dstR[i], tmp);
        dstR[i] = _mm_cvtps_epi32(dstR[i]);
        
        dstG[i] = _mm_add_ps(dstG[i], tmpY);
        tmp = _mm_mul_ps(tmpU, U2G_s);
        dstG[i] = _mm_add_ps(dstG[i], tmp);
        tmp = _mm_mul_ps(tmpV, V2G_s);
        dstG[i] = _mm_add_ps(dstG[i], tmp);
        dstG[i] = _mm_cvtps_epi32(dstG[i]);
        
        dstB[i] = _mm_add_ps(dstB[i], tmpY);
        tmp = _mm_mul_ps(tmpU, U2B_s);
        dstB[i] = _mm_add_ps(dstB[i], tmp);
        dstB[i] = _mm_cvtps_epi32(dstB[i]);

    }
    rgb.round();
    rgb.s32_to_u8();
    rgb.update_32_16();
    _mm_empty();
}

/*
 * Blending
 */
void blending_with_sse(RGB & rgb_blending, const RGB & rgb1, const RGB & rgb2, const int A, const bool mode){
    _mm_empty();
    
    __m128i *RR = (__m128i *)rgb_blending.pR32;
    __m128i *GG = (__m128i *)rgb_blending.pG32;
    __m128i *BB = (__m128i *)rgb_blending.pB32;
    
    __m128i *RR1 = (__m128i *)rgb1.pR32;
    __m128i *GG1 = (__m128i *)rgb1.pG32;
    __m128i *BB1 = (__m128i *)rgb1.pB32;
    __m128i alpha128 = _mm_set1_ps((float)A);
    int64_t nloop = rgb_blending.size >> 2;
    __m128 tmp1, tmp2;
    __m128i tmpi;
    
    if(mode){
        __m128i *RR2 = (__m128i *)rgb2.pR32;
        __m128i *GG2 = (__m128i *)rgb2.pG32;
        __m128i *BB2 = (__m128i *)rgb2.pB32;
        int _A = 256 - A;
        __m128 _alpha128 = _mm_set1_ps((float)_A);
        for(int i = 0; i < nloop; i++){
            tmp1 = _mm_cvtepi32_ps(RR1[i]);
            tmp1 = _mm_mul_ps(tmp1, alpha128);
            tmpi = _mm_cvtps_epi32(tmp1);
            RR[i] = _mm_srai_epi32(tmpi, 8);
            
            tmp2 = _mm_cvtepi32_ps(RR2[i]);
            tmp2 = _mm_mul_ps(tmp2, _alpha128);
            tmpi = _mm_cvtps_epi32(tmp2);
            tmpi = _mm_srai_epi32(tmpi, 8);
            RR[i] = _mm_add_epi32(RR[i], tmpi);
        }
        
        for(int i = 0; i < nloop; i++){
            tmp1 = _mm_cvtepi32_ps(GG1[i]);
            tmp1 = _mm_mul_ps(tmp1, alpha128);
            tmpi = _mm_cvtps_epi32(tmp1);
            GG[i] = _mm_srai_epi32(tmpi, 8);
            
            tmp2 = _mm_cvtepi32_ps(GG2[i]);
            tmp2 = _mm_mul_ps(tmp2, _alpha128);
            tmpi = _mm_cvtps_epi32(tmp2);
            tmpi = _mm_srai_epi32(tmpi, 8);
            GG[i] = _mm_add_epi32(GG[i], tmpi);
        }
        
        for(int i = 0; i < nloop; i++){
            tmp1 = _mm_cvtepi32_ps(BB1[i]);
            tmp1 = _mm_mul_ps(tmp1, alpha128);
            tmpi = _mm_cvtps_epi32(tmp1);
            BB[i] = _mm_srai_epi32(tmpi, 8);
            
            tmp2 = _mm_cvtepi32_ps(BB2[i]);
            tmp2 = _mm_mul_ps(tmp2, _alpha128);
            tmpi = _mm_cvtps_epi32(tmp2);
            tmpi = _mm_srai_epi32(tmpi, 8);
            BB[i] = _mm_add_epi32(BB[i], tmpi);
        }
    }else {
        for(int i = 0; i < nloop; i++){
            tmp1 = _mm_cvtepi32_ps(RR1[i]);
            tmp1 = _mm_mul_ps(tmp1, alpha128);
            tmpi = _mm_cvtps_epi32(tmp1);
            RR[i] = _mm_srai_epi32(tmpi, 8);
        }
        
        for(int i = 0; i < nloop; i++){
            tmp1 = _mm_cvtepi32_ps(GG1[i]);
            tmp1 = _mm_mul_ps(tmp1, alpha128);
            tmpi = _mm_cvtps_epi32(tmp1);
            GG[i] = _mm_srai_epi32(tmpi, 8);
        }

        for(int i = 0; i < nloop; i++){
            tmp1 = _mm_cvtepi32_ps(BB1[i]);
            tmp1 = _mm_mul_ps(tmp1, alpha128);
            tmpi = _mm_cvtps_epi32(tmp1);
            BB[i] = _mm_srai_epi32(tmpi, 8);
        }

    }
    rgb_blending.s32_to_u8();
    rgb_blending.update_32_16();

}

/*
 * rgb2yuv
 * Y= 0.256788*R + 0.504129*G + 0.097906*B + 16
 * U= -0.148223*R - 0.290993*G + 0.439216*B + 128
 * V= 0.439216*R - 0.367788*G - 0.071427*B + 128
 */
void rgb2yuv_with_sse(YUV & yuv,const RGB & rgb){
    int32_t * supR = new int32_t[yuv.size >> 2];
    int32_t * supG = new int32_t[yuv.size >> 2];
    int32_t * supB = new int32_t[yuv.size >> 2];
    
//    rgb.write(foutcheck2);
    
    // Supress RGB to match U/V
    for(int row = 0, iUV = 0; row < yuv.height; row += 2){
        for(int col = 0; col < yuv.width; col += 2, iUV++){
            int iY = row * yuv.width + col;
            supR[iUV] = rgb.pR32[iY];
            supG[iUV] = rgb.pG32[iY];
            supB[iUV] = rgb.pB32[iY];
         }
     }

    _mm_empty();
    
    __m128 * dstY = (__m128*) yuv.pY32;
    __m128 * dstU = (__m128*) yuv.pU32;
    __m128 * dstV = (__m128*) yuv.pV32;
    __m128 * srcR = (__m128*) rgb.pR32;
    __m128 * srcG = (__m128*) rgb.pG32;
    __m128 * srcB = (__m128*) rgb.pB32;


    __m128 tmp;
    
    int64_t nloop = yuv.size >> 2;
    // Get Y
    for(int i = 0; i < nloop; i++){
        dstY[i] = _mm_setzero_ps();

        srcR[i] = _mm_cvtepi32_ps(srcR[i]);
        srcG[i] = _mm_cvtepi32_ps(srcG[i]);
        srcB[i] = _mm_cvtepi32_ps(srcB[i]);
  
        tmp = _mm_mul_ps(srcR[i], R2Y_s);
        dstY[i] = _mm_add_ps(dstY[i], tmp);
        tmp = _mm_mul_ps(srcG[i], G2Y_s);
        dstY[i] = _mm_add_ps(dstY[i], tmp);
        tmp = _mm_mul_ps(srcB[i], B2Y_s);
        dstY[i] = _mm_add_ps(dstY[i], tmp);
        
        dstY[i] = _mm_cvtps_epi32(dstY[i]);
    }
    
    __m128 * srcRR = (__m128*) supR;
    __m128 * srcGG = (__m128*) supG;
    __m128 * srcBB = (__m128*) supB;

    int64_t nloopUV = yuv.size >> 4;
    for(int i = 0; i < nloopUV; i++){
        dstU[i] = _mm_setzero_ps();
        dstV[i] = _mm_setzero_ps();

        srcRR[i] = _mm_cvtepi32_ps(srcRR[i]);
        srcGG[i] = _mm_cvtepi32_ps(srcGG[i]);
        srcBB[i] = _mm_cvtepi32_ps(srcBB[i]);

        
        tmp = _mm_mul_ps(srcRR[i], R2U_s);
        dstU[i] = _mm_add_ps(dstU[i], tmp);
        tmp = _mm_mul_ps(srcGG[i], G2U_s);
        dstU[i] = _mm_add_ps(dstU[i], tmp);
        tmp = _mm_mul_ps(srcBB[i], B2U_s);
        dstU[i] = _mm_add_ps(dstU[i], tmp);
        dstU[i] = _mm_cvtps_epi32(dstU[i]);

        
        tmp = _mm_mul_ps(srcRR[i], R2V_s);
        dstV[i] = _mm_add_ps(dstV[i], tmp);
        tmp = _mm_mul_ps(srcGG[i], G2V_s);
        dstV[i] = _mm_add_ps(dstV[i], tmp);
        tmp = _mm_mul_ps(srcBB[i], B2V_s);
        dstV[i] = _mm_add_ps(dstV[i], tmp);
        dstV[i] = _mm_cvtps_epi32(dstV[i]);
    }
    
    yuv.s32_to_u8();
    _mm_empty();

}


int process_with_sse(YUV &OUT_YUV, const YUV &DEM1_YUV, const YUV &DEM2_YUV, RGB &CHECK_RGB1, RGB &CHECK_RGB2, const bool mode){
    clock_t begin_time = clock();
    clock_t total_time = 0;
    
    RGB rgb_blending = RGB(DEM1_YUV.width, DEM1_YUV.height);
    
    cout << "\nSSE..." << endl;
    clock_t core_time = clock();
    
    yuv2rgb_without_simd(DEM1_YUV, CHECK_RGB1);
    //yuv2rgb_with_sse(DEM1_YUV, CHECK_RGB1);
    if(mode)
        yuv2rgb_without_simd(DEM2_YUV, CHECK_RGB2);
        //yuv2rgb_with_sse(DEM2_YUV, CHECK_RGB2);
    
    CHECK_RGB1.write(foutcheck1);
    
    for (int A = 1; A < 256; A += 3) {
        blending_with_sse(rgb_blending, CHECK_RGB1, CHECK_RGB2, A, mode);
        
        if(A == 253)
            rgb_blending.write(foutcheck2);
        
//        rgb2yuv_without_simd(OUT_YUV, rgb_blending);
        rgb2yuv_with_sse(OUT_YUV, rgb_blending);
        
        total_time += clock() - core_time;
        core_time = clock();
        
        if(mode)
            OUT_YUV.write(fout22);
        else
            OUT_YUV.write(fout21);
    }// process end
    if(mode)    fclose(fout22);
    else    fclose(fout21);
    
    if(mode){
        cout << "Alpha Blending with dem1.yuv and dem2.yuv, output file is \"alpha1-2.yuv\":" <<endl;
        cout << "Core function time: " << (double)total_time / CLOCKS_PER_SEC * 1000<< "ms" << endl;
        cout << "Include output time: " << (double)(clock() - begin_time) / CLOCKS_PER_SEC * 1000 << "ms" << endl << endl;
    }else{
        cout << "Alpha Blending with dem1.yuv, output file is \"alpha1-1.yuv\":" <<endl;
        cout << "Core function time: " << (double)total_time / CLOCKS_PER_SEC * 1000<< "ms" << endl;
        cout << "Include output time: " << (double)(clock() - begin_time) / CLOCKS_PER_SEC * 1000 << "ms" << endl << endl;
    }

    return 0;
}
