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


/*
 * yuv2rgb
 * R = 1.164383 * (Y - 16) + 1.596027*(V - 128)
 * G = 1.164383 * (Y - 16) – 0.391762*(U - 128) – 0.812968*(V - 128)
 * B = 1.164383 * (Y - 16) + 2.017232*(U - 128)
 */
void yuv2rgb_with_avx(const YUV & yuv, RGB & rgb){
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
    
    __m256 tmp, tmpY, tmpU, tmpV;
    __m256 * dstR = (__m256*) rgb.pR32;
    __m256 * dstG = (__m256*) rgb.pG32;
    __m256 * dstB = (__m256*) rgb.pB32;
    
    __m256 * Yp32 = new __m256[yuv.size >> 3];
    __m256 * Up32 = (__m256 *)dupPU32;
    __m256 * Vp32 = (__m256 *)dupPV32;
    
    int64_t nloop = yuv.size >> 3;
    for(int i = 0; i < nloop; i++){
        int ii = i << 3;
        Yp32[i] = _mm256_set_ps((float)yuv.pY32[ii], (float)yuv.pY32[ii + 1],
                                (float)yuv.pY32[ii + 2], (float)yuv.pY32[ii + 3],
                                (float)yuv.pY32[ii + 4], (float)yuv.pY32[ii + 5],
                                (float)yuv.pY32[ii + 6], (float)yuv.pY32[ii + 7]);
        Up32[i] = _mm256_cvtepi32_ps(Up32[i]);
        Vp32[i] = _mm256_cvtepi32_ps(Vp32[i]);
        
        dstR[i] = _mm256_setzero_ps();
        dstG[i] = _mm256_setzero_ps();
        dstB[i] = _mm256_setzero_ps();
        
        tmpY = _mm256_sub_ps(Yp32[i], YConst16_256s);
        tmpY = _mm256_mul_ps(tmpY, Y2RGB_256s);           // tmpY = 1.164383 * (Y - 16)
        tmpU = _mm256_sub_ps(Up32[i], UVConst128_256s);   // tmpU = (U - 128)
        tmpV = _mm256_sub_ps(Vp32[i], UVConst128_256s);   // tmpV = (V - 128)
        
        dstR[i] = _mm256_add_ps(dstR[i], tmpY);
        tmp = _mm256_mul_ps(tmpV, V2R_256s);
        dstR[i] = _mm256_add_ps(dstR[i], tmp);
        dstR[i] = _mm256_cvtps_epi32(dstR[i]);
        
        dstG[i] = _mm256_add_ps(dstG[i], tmpY);
        tmp = _mm256_mul_ps(tmpU, U2G_256s);
        dstG[i] = _mm256_add_ps(dstG[i], tmp);
        tmp = _mm256_mul_ps(tmpV, V2G_256s);
        dstG[i] = _mm256_add_ps(dstG[i], tmp);
        dstG[i] = _mm256_cvtps_epi32(dstG[i]);
        
        dstB[i] = _mm256_add_ps(dstB[i], tmpY);
        tmp = _mm256_mul_ps(tmpU, U2B_256s);
        dstB[i] = _mm256_add_ps(dstB[i], tmp);
        dstB[i] = _mm256_cvtps_epi32(dstB[i]);
        
    }
    rgb.round();
    rgb.s32_to_u8();
    rgb.update_32_16();
    
    delete [] dupPU32;
    delete [] dupPV32;
    delete [] Yp32;
    
    _mm_empty();

}
void blending_with_avx(RGB & rgb_blending, const RGB & rgb1, const RGB & rgb2, const int A, const bool mode){
    _mm_empty();
    
    __m256 *RR = (__m256 *)rgb_blending.pR32;
    __m256 *GG = (__m256 *)rgb_blending.pG32;
    __m256 *BB = (__m256 *)rgb_blending.pB32;
    
    __m256 *RR1 = (__m256 *)rgb1.pR32;
    __m256 *GG1 = (__m256 *)rgb1.pG32;
    __m256 *BB1 = (__m256 *)rgb1.pB32;
    __m256 alpha256 = _mm256_set1_ps((float)A);
    int64_t nloop = rgb_blending.size >> 3;
    __m256 tmp1, tmp2;
    __m256i tmpi;
    
    if(mode){
        __m256 *RR2 = (__m256 *)rgb2.pR32;
        __m256 *GG2 = (__m256 *)rgb2.pG32;
        __m256 *BB2 = (__m256 *)rgb2.pB32;
        int _A = 256 - A;
        __m256 _alpha256 = _mm256_set1_ps((float)_A);
        for(int i = 0; i < nloop; i++){
            tmp1 = _mm256_cvtepi32_ps(RR1[i]);
            tmp1 = _mm256_mul_ps(tmp1, alpha256);
            tmpi = _mm256_cvtps_epi32(tmp1);
            RR[i] = _mm256_srai_epi32(tmpi, 8);
            
            tmp2 = _mm256_cvtepi32_ps(RR2[i]);
            tmp2 = _mm256_mul_ps(tmp2, _alpha256);
            tmpi = _mm256_cvtps_epi32(tmp2);
            tmpi = _mm256_srai_epi32(tmpi, 8);
            RR[i] = _mm256_add_epi32(RR[i], tmpi);
        }
        
        for(int i = 0; i < nloop; i++){
            tmp1 = _mm256_cvtepi32_ps(GG1[i]);
            tmp1 = _mm256_mul_ps(tmp1, alpha256);
            tmpi = _mm256_cvtps_epi32(tmp1);
            GG[i] = _mm256_srai_epi32(tmpi, 8);
            
            tmp2 = _mm256_cvtepi32_ps(GG2[i]);
            tmp2 = _mm256_mul_ps(tmp2, _alpha256);
            tmpi = _mm256_cvtps_epi32(tmp2);
            tmpi = _mm256_srai_epi32(tmpi, 8);
            GG[i] = _mm256_add_epi32(GG[i], tmpi);
        }
        
        for(int i = 0; i < nloop; i++){
            tmp1 = _mm256_cvtepi32_ps(BB1[i]);
            tmp1 = _mm256_mul_ps(tmp1, alpha256);
            tmpi = _mm256_cvtps_epi32(tmp1);
            BB[i] = _mm256_srai_epi32(tmpi, 8);
            
            tmp2 = _mm256_cvtepi32_ps(BB2[i]);
            tmp2 = _mm256_mul_ps(tmp2, _alpha256);
            tmpi = _mm256_cvtps_epi32(tmp2);
            tmpi = _mm256_srai_epi32(tmpi, 8);
            BB[i] = _mm256_add_epi32(BB[i], tmpi);
        }
    }else {
        for(int i = 0; i < nloop; i++){
            tmp1 = _mm256_cvtepi32_ps(RR1[i]);
            tmp1 = _mm256_mul_ps(tmp1, alpha256);
            tmpi = _mm256_cvtps_epi32(tmp1);
            RR[i] = _mm256_srai_epi32(tmpi, 8);
        }
        
        for(int i = 0; i < nloop; i++){
            tmp1 = _mm256_cvtepi32_ps(GG1[i]);
            tmp1 = _mm256_mul_ps(tmp1, alpha256);
            tmpi = _mm256_cvtps_epi32(tmp1);
            GG[i] = _mm256_srai_epi32(tmpi, 8);
        }
        
        for(int i = 0; i < nloop; i++){
            tmp1 = _mm256_cvtepi32_ps(BB1[i]);
            tmp1 = _mm256_mul_ps(tmp1, alpha256);
            tmpi = _mm256_cvtps_epi32(tmp1);
            BB[i] = _mm256_srai_epi32(tmpi, 8);
        }
        
    }
    rgb_blending.s32_to_u8();
    rgb_blending.update_32_16();

}
void rgb2yuv_with_avx(YUV & yuv,const RGB & rgb){
    int32_t * supR = new int32_t[yuv.size >> 2];
    int32_t * supG = new int32_t[yuv.size >> 2];
    int32_t * supB = new int32_t[yuv.size >> 2];
    

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
    
    __m256 * dstY = (__m256*) yuv.pY32;
    __m256 * dstU = (__m256*) yuv.pU32;
    __m256 * dstV = (__m256*) yuv.pV32;
    __m256 * srcR = new __m256[rgb.size >> 3];
    __m256 * srcG = new __m256[rgb.size >> 3];
    __m256 * srcB = new __m256[rgb.size >> 3];
    
    
    __m256 tmp;
    
    int64_t nloop = yuv.size >> 3;
    // Get Y
    for(int i = 0; i < nloop; i++){
        dstY[i] = _mm256_setzero_ps();
        
        int ii = i << 3;
        srcR[i] = _mm256_set_ps((float)rgb.pR32[ii], (float)rgb.pR32[ii + 1],
                                (float)rgb.pR32[ii + 2], (float)rgb.pR32[ii + 3],
                                (float)rgb.pR32[ii + 4], (float)rgb.pR32[ii + 5],
                                (float)rgb.pR32[ii + 6], (float)rgb.pR32[ii + 7]);
        srcG[i] = _mm256_set_ps((float)rgb.pG32[ii], (float)rgb.pG32[ii + 1],
                                (float)rgb.pG32[ii + 2], (float)rgb.pG32[ii + 3],
                                (float)rgb.pG32[ii + 4], (float)rgb.pG32[ii + 5],
                                (float)rgb.pG32[ii + 6], (float)rgb.pG32[ii + 7]);
        srcB[i] = _mm256_set_ps((float)rgb.pB32[ii], (float)rgb.pB32[ii + 1],
                                (float)rgb.pB32[ii + 2], (float)rgb.pB32[ii + 3],
                                (float)rgb.pB32[ii + 4], (float)rgb.pB32[ii + 5],
                                (float)rgb.pB32[ii + 6], (float)rgb.pB32[ii + 7]);
        tmp = _mm256_mul_ps(srcR[i], R2Y_256s);
        dstY[i] = _mm256_add_ps(dstY[i], tmp);
        tmp = _mm256_mul_ps(srcG[i], G2Y_256s);
        dstY[i] = _mm256_add_ps(dstY[i], tmp);
        tmp = _mm256_mul_ps(srcB[i], B2Y_256s);
        dstY[i] = _mm256_add_ps(dstY[i], tmp);
        dstY[i] = _mm256_add_ps(dstY[i], YConst16_256s);
        
        dstY[i] = _mm256_cvtps_epi32(dstY[i]);
    }
    
    delete [] srcR;
    delete [] srcG;
    delete [] srcB;
    
    __m256 * srcRR = (__m256*) supR;
    __m256 * srcGG = (__m256*) supG;
    __m256 * srcBB = (__m256*) supB;
    
    int64_t nloopUV = yuv.size >> 5;
    for(int i = 0; i < nloopUV; i++){
        dstU[i] = _mm256_setzero_ps();
        dstV[i] = _mm256_setzero_ps();
        
        srcRR[i] = _mm256_cvtepi32_ps(srcRR[i]);
        srcGG[i] = _mm256_cvtepi32_ps(srcGG[i]);
        srcBB[i] = _mm256_cvtepi32_ps(srcBB[i]);
        
        
        tmp = _mm256_mul_ps(srcRR[i], R2U_256s);
        dstU[i] = _mm256_add_ps(dstU[i], tmp);
        tmp = _mm256_mul_ps(srcGG[i], G2U_256s);
        dstU[i] = _mm256_add_ps(dstU[i], tmp);
        tmp = _mm256_mul_ps(srcBB[i], B2U_256s);
        dstU[i] = _mm256_add_ps(dstU[i], tmp);
        dstU[i] = _mm256_add_ps(dstU[i], UVConst128_256s);
        dstU[i] = _mm256_cvtps_epi32(dstU[i]);
        
        
        tmp = _mm256_mul_ps(srcRR[i], R2V_256s);
        dstV[i] = _mm256_add_ps(dstV[i], tmp);
        tmp = _mm256_mul_ps(srcGG[i], G2V_256s);
        dstV[i] = _mm256_add_ps(dstV[i], tmp);
        tmp = _mm256_mul_ps(srcBB[i], B2V_256s);
        dstV[i] = _mm256_add_ps(dstV[i], tmp);
        dstV[i] = _mm256_add_ps(dstV[i], UVConst128_256s);
        dstV[i] = _mm256_cvtps_epi32(dstV[i]);
    }
    
    
    yuv.s32_to_u8();
    
    delete [] supR;
    delete [] supG;
    delete [] supB;
    
    _mm_empty();

}


int process_with_avx(YUV &OUT_YUV, const YUV &DEM1_YUV, const YUV &DEM2_YUV, RGB &CHECK_RGB1, RGB &CHECK_RGB2, const bool mode){
    clock_t begin_time = clock();
    clock_t total_time = 0;
    
    RGB rgb_blending = RGB(DEM1_YUV.width, DEM1_YUV.height);
    
    cout << "\nAVX..." << endl;
    clock_t core_time = clock();
    
    yuv2rgb_with_avx(DEM1_YUV, CHECK_RGB1);
    if(mode)
        yuv2rgb_with_avx(DEM2_YUV, CHECK_RGB2);
    
    CHECK_RGB1.write(foutcheck1);
    
    for (int A = 1; A < 256; A += 3) {
        blending_with_avx(rgb_blending, CHECK_RGB1, CHECK_RGB2, A, mode);
        if(A == 253)
            rgb_blending.write(foutcheck2);
        
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
    
    return 0;
}
