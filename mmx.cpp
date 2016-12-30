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


using namespace std;

extern FILE *fout11, *fout12;
extern FILE *foutcheck1, *foutcheck2;

extern void yuv2rgb_without_simd(const YUV & yuv, RGB & rgb);
extern void blending_without_simd(RGB & rgb_blending, const RGB & rgb1, const RGB & rgb2, const int A, const bool mode);
extern void rgb2yuv_without_simd(YUV & yuv,const RGB & rgb);

/*
 * R = 1.164383 * (Y - 16) + 1.596027*(V - 128)
 * G = 1.164383 * (Y - 16) – 0.391762*(U - 128) – 0.812968*(V - 128)
 * B = 1.164383 * (Y - 16) + 2.017232*(U - 128)
 */
// const int16_t YUV2RGB_OFFSET[3][3] = {{1, 0, 1},{1, 0, 0},{1, 2, 0}};
const __m64 YConst16 = _mm_set_pi16(16, 16, 16, 16);
const __m64 UVConst128 = _mm_set_pi16(128, 128, 128, 128);
const int16_t YUV2R[3] = {(int16_t)0.164383*(1<<16),                         0, (int16_t)(0.596027*(1<<16))};
const int16_t YUV2G[3] = {(int16_t)0.164383*(1<<16), (int16_t)(-0.391762*(1<<16)), (int16_t)(-0.812968*(1<<16))};
const int16_t YUV2B[3] = {(int16_t)0.164383*(1<<16),  (int16_t)(0.017232*(1<<16)),                         0};
const __m64 Y2RGB = _mm_set_pi16(YUV2R[0], YUV2R[0], YUV2R[0], YUV2R[0]);       // 0.164383
const __m64 V2R = _mm_set_pi16(YUV2R[2], YUV2R[2], YUV2R[2], YUV2R[2]);         // 0.596027
const __m64 U2G = _mm_set_pi16(YUV2G[1], YUV2G[1], YUV2G[1], YUV2G[1]);         //-0.391762
const __m64 V2G = _mm_set_pi16(YUV2G[2], YUV2G[2], YUV2G[2], YUV2G[2]);         //-0.812968
const __m64 U2B = _mm_set_pi16(YUV2B[1], YUV2B[1], YUV2B[1], YUV2B[1]);         // 0.017232

/* 
 * Y= 0.256788*R + 0.504129*G + 0.097906*B + 16
 * U= -0.148223*R - 0.290993*G + 0.439216*B + 128
 * V= 0.439216*R - 0.367788*G - 0.071427*B + 128
 */
const int16_t RGB2Y[3] = {(int16_t)(0.256788*(1<<16)), (int16_t)(0.504129*(1<<16)), (int16_t)(0.097906*(1<<16))};
const int16_t RGB2U[3] = {(int16_t)(-0.148223*(1<<16)), (int16_t)(-0.290993*(1<<16)), (int16_t)(0.439216*(1<<16))};
const int16_t RGB2V[3] = {(int16_t)(0.439216*(1<<16)), (int16_t)(-0.367788*(1<<16)), (int16_t)(-0.071427*(1<<16))};
const __m64 R2Y = _mm_set_pi16(RGB2Y[0], RGB2Y[0], RGB2Y[0], RGB2Y[0]);         // 0.299
const __m64 G2Y = _mm_set_pi16(RGB2Y[1], RGB2Y[1], RGB2Y[1], RGB2Y[1]);         // 0.587
const __m64 B2Y = _mm_set_pi16(RGB2Y[2], RGB2Y[2], RGB2Y[2], RGB2Y[2]);         // 0.114
const __m64 R2U = _mm_set_pi16(RGB2U[0], RGB2U[0], RGB2U[0], RGB2U[0]);         //-0.147
const __m64 G2U = _mm_set_pi16(RGB2U[1], RGB2U[1], RGB2U[1], RGB2U[1]);         //-0.289
const __m64 B2U = _mm_set_pi16(RGB2U[2], RGB2U[2], RGB2U[2], RGB2U[2]);         // 0.436
const __m64 R2V = _mm_set_pi16(RGB2V[0], RGB2V[0], RGB2V[0], RGB2V[0]);         // 0.615
const __m64 G2V = _mm_set_pi16(RGB2V[1], RGB2V[1], RGB2V[1], RGB2V[1]);         //-0.515
const __m64 B2V = _mm_set_pi16(RGB2V[2], RGB2V[2], RGB2V[2], RGB2V[2]);         //-0.100

/*
 * YUV2RGB
 * R = 1.164383 * (Y - 16) + 1.596027*(V - 128)
 * G = 1.164383 * (Y - 16) – 0.391762*(U - 128) – 0.812968*(V - 128)
 * B = 1.164383 * (Y - 16) + 2.017232*(U - 128)
 * const int16_t YUV2RGB_OFFSET[3][3] = {{1, 0, 1},{1, 0, 0},{1, 2, 0}};
 * V->R, U,V->G, U->B
 */
void yuv2rgb_with_mmx(const YUV & yuv, RGB & rgb){
    _mm_empty();
    
    __m64 tmp, tmpY, tmpU, tmpV;
    __m64 * dstR = (__m64*) rgb.pR16;
    __m64 * dstG = (__m64*) rgb.pG16;
    __m64 * dstB = (__m64*) rgb.pB16;
    
    
    
    // duplicate U and V, preprocess for pack computing
    int16_t *dupPU16 = new int16_t[yuv.size];
    int16_t *dupPV16 = new int16_t[yuv.size];
    for(int i = 0; i < yuv.size; i++){
        int iUV = yuv.getBlockID(i);
        dupPU16[i] = yuv.pU16[iUV];
        dupPV16[i] = yuv.pV16[iUV];
    }
    
    __m64 * Yp16 = (__m64 *)yuv.pY16;
    __m64 * Up16 = (__m64 *)dupPU16;
    __m64 * Vp16 = (__m64 *)dupPV16;
    
    
    int64_t nloop = yuv.size >> 2;
    for(int i = 0; i < nloop; i++){
        tmp = _mm_setzero_si64();
        dstR[i] = _mm_setzero_si64();
        dstG[i] = _mm_setzero_si64();
        dstB[i] = _mm_setzero_si64();
        
        tmpY = _mm_sub_pi16(Yp16[i], YConst16);         // (Y-16)
        tmpU = _mm_sub_pi16(Up16[i], UVConst128);       // (U-128)
        tmpV = _mm_sub_pi16(Vp16[i], UVConst128);       // (V-128)

        dstR[i] = _mm_add_pi16(dstR[i], tmpY);          // R = (Y - 16) + ...
        dstG[i] = _mm_add_pi16(dstG[i], tmpY);          // G = (Y - 16) + ...
        dstB[i] = _mm_add_pi16(dstB[i], tmpY);          // B = (Y - 16) + ...
        tmp = _mm_mulhi_pi16(tmpY, Y2RGB);
        dstR[i] = _mm_add_pi16(dstR[i], tmp);           // R = 1.164383 * (Y - 16) + ...
        dstG[i] = _mm_add_pi16(dstG[i], tmp);           // G = 1.164383 * (Y - 16) + ...
        dstB[i] = _mm_add_pi16(dstB[i], tmp);           // B = 1.164383 * (Y - 16) + ...
        
        // Get R
        dstR[i] = _mm_add_pi16(dstR[i], tmpV);          // R += (V-128)
        tmp = _mm_mulhi_pi16(tmpV, V2R);                // 0.596027*(V-128)
        dstR[i] = _mm_add_pi16(dstR[i], tmp);           // R += 1.596027*(V-128)
        
        // Get G
        tmp = _mm_mulhi_pi16(tmpU, U2G);
        dstG[i] = _mm_add_pi16(dstG[i], tmp);
        tmp = _mm_mulhi_pi16(tmpV, V2G);
        dstG[i] = _mm_add_pi16(dstG[i], tmp);
        
        // Get B
        tmp = _mm_mulhi_pi16(tmpU, U2B);
        dstB[i] = _mm_add_pi16(dstB[i], tmp);
        tmp = _mm_slli_pi16(tmpU, 1);
        dstB[i] = _mm_add_pi16(dstB[i], tmp);
    }
    rgb.round();

    _mm_empty();

}

/*
 * Blending
 */
void blending_with_mmx(RGB & rgb_blending, const RGB & rgb1, const RGB & rgb2, const int A, const bool mode){
    _mm_empty();

    __m64 *RR = (__m64 *)rgb_blending.pR16;
    __m64 *GG = (__m64 *)rgb_blending.pG16;
    __m64 *BB = (__m64 *)rgb_blending.pB16;

    __m64 *RR1 = (__m64 *)rgb1.pR16;
    __m64 *GG1 = (__m64 *)rgb1.pG16;
    __m64 *BB1 = (__m64 *)rgb1.pB16;
    __m64 alpha64 = _mm_set_pi16((int16_t)A, (int16_t)A, (int16_t)A, (int16_t)A);
    int64_t nloop = rgb_blending.size >> 2;
    __m64 tmp1, tmp2;

    if(mode){
        __m64 *RR2 = (__m64 *)rgb2.pR16;
        __m64 *GG2 = (__m64 *)rgb2.pG16;
        __m64 *BB2 = (__m64 *)rgb2.pB16;
        int _A = 256 - A;
        __m64 _alpha64 = _mm_set_pi16((int16_t)_A, (int16_t)_A, (int16_t)_A, (int16_t)_A);
        for(int i = 0; i < nloop; i++){
            tmp1 = _mm_slli_pi16(RR1[i], 2);
            tmp2 = _mm_slli_pi16(alpha64, 6);
            RR[i] = _mm_mulhi_pi16(tmp1, tmp2);
            
            tmp1 = _mm_slli_pi16(RR2[i], 2);
            tmp2 = _mm_slli_pi16(_alpha64, 6);
            tmp1 = _mm_mulhi_pi16(tmp1, tmp2);
            RR[i] = _mm_add_pi16(RR[i], tmp1);
        }
        for(int i = 0; i < nloop; i++){
            tmp1 = _mm_slli_pi16(GG1[i], 2);
            tmp2 = _mm_slli_pi16(alpha64, 6);
            GG[i] = _mm_mulhi_pi16(tmp1, tmp2);
            
            tmp1 = _mm_slli_pi16(GG2[i], 2);
            tmp2 = _mm_slli_pi16(_alpha64, 6);
            tmp1 = _mm_mulhi_pi16(tmp1, tmp2);
            GG[i] = _mm_add_pi16(GG[i], tmp1);
        }
        for(int i = 0; i < nloop; i++){
            tmp1 = _mm_slli_pi16(BB1[i], 2);
            tmp2 = _mm_slli_pi16(alpha64, 6);
            BB[i] = _mm_mulhi_pi16(tmp1, tmp2);

            tmp1 = _mm_slli_pi16(BB2[i], 2);
            tmp2 = _mm_slli_pi16(_alpha64, 6);
            tmp1 = _mm_mulhi_pi16(tmp1, tmp2);
            BB[i] = _mm_add_pi16(BB[i], tmp1);
         }
    }
    else{
        for(int i = 0; i < nloop; i++){
            //tmp1 = _mm_mullo_pi16(RR1[i], alpha64);
            //RR[i] = _mm_srli_pi16(tmp1, 8);

            tmp1 = _mm_slli_pi16(RR1[i], 2);
            tmp2 = _mm_slli_pi16(alpha64, 6);
            RR[i] = _mm_mulhi_pi16(tmp1, tmp2);
        }
        for(int i = 0; i < nloop; i++){
            //tmp1 = _mm_mullo_pi16(GG1[i], alpha64);
            //GG[i] = _mm_srli_pi16(tmp1, 8);

            tmp1 = _mm_slli_pi16(GG1[i], 2);
            tmp2 = _mm_slli_pi16(alpha64, 6);
            GG[i] = _mm_mulhi_pi16(tmp1, tmp2);
        }
        for(int i = 0; i < nloop; i++){
            //tmp1 = _mm_mullo_pi16(BB1[i], alpha64);
            //BB[i] = _mm_srli_pi16(tmp1, 8);

            tmp1 = _mm_slli_pi16(BB1[i], 2);
            tmp2 = _mm_slli_pi16(alpha64, 6);
            BB[i] = _mm_mulhi_pi16(tmp1, tmp2);
        }
    }
    _mm_empty();
}

/* 
 * RGB2YUV
 * Y= 0.256788*R + 0.504129*G + 0.097906*B + 16
 * U= -0.148223*R - 0.290993*G + 0.439216*B + 128
 * V= 0.439216*R - 0.367788*G - 0.071427*B + 128
 */
void rgb2yuv_with_mmx(YUV & yuv, const RGB & rgb){
    _mm_empty();
    
    int64_t nloop = yuv.size >> 2;
    __m64 * dstY = (__m64*) yuv.pY16;
    __m64 * dstU = (__m64*) yuv.pU16;
    __m64 * dstV = (__m64*) yuv.pV16;
    __m64 * srcR = (__m64*) rgb.pR16;
    __m64 * srcG = (__m64*) rgb.pG16;
    __m64 * srcB = (__m64*) rgb.pB16;
    int16_t * supR = new int16_t[nloop];
    int16_t * supG = new int16_t[nloop];
    int16_t * supB = new int16_t[nloop];
    __m64 tmp;
    
    // Get Y
    for(int i = 0; i < nloop; i++){
        dstY[i] = _mm_setzero_si64();
        tmp = _mm_mulhi_pi16(srcR[i], R2Y);                  // 0.299*RR + 0.587*GG + 0.114*BB
        dstY[i] = _mm_add_pi16(dstY[i], tmp);
        tmp = _mm_mulhi_pi16(srcG[i], G2Y);
        dstY[i] = _mm_add_pi16(dstY[i], tmp);
        tmp = _mm_mulhi_pi16(srcB[i], B2Y);
        dstY[i] = _mm_add_pi16(dstY[i], tmp);
        dstY[i] = _mm_add_pi16(dstY[i], YConst16);
    }
    
    
    // Supress RGB to match U/V
    for(int row = 0, iUV = 0; row < yuv.height; row += 2){
        for(int col = 0; col < yuv.width; col += 2, iUV++){
            int iY = row * yuv.width + col;
            supR[iUV] = rgb.pR16[iY];
            supG[iUV] = rgb.pG16[iY];
            supB[iUV] = rgb.pB16[iY];
        }
    }
    srcR = (__m64*) supR;
    srcG = (__m64*) supG;
    srcB = (__m64*) supB;
    int64_t nloopUV = yuv.size >> 4;
    for(int i = 0; i < nloopUV; i++){
        dstU[i] = _mm_setzero_si64();
        dstV[i] = _mm_setzero_si64();
        
        // Get U
        tmp = _mm_mulhi_pi16(srcR[i], R2U);              // -0.147*RR - 0.289*GG + 0.436*BB + 128;
        dstU[i] = _mm_add_pi16(dstU[i], tmp);
        tmp = _mm_mulhi_pi16(srcG[i], G2U);
        dstU[i] = _mm_add_pi16(dstU[i], tmp);
        tmp = _mm_mulhi_pi16(srcB[i], B2U);
        dstU[i] = _mm_add_pi16(dstU[i], tmp);
        dstU[i] = _mm_add_pi16(dstU[i], UVConst128);
        
        // Get V
        tmp = _mm_mulhi_pi16(srcR[i], R2V);             // 0.615*RR - 0.515*GG - 0.100*BB + 128;
        dstV[i] = _mm_add_pi16(dstV[i], tmp);
        tmp = _mm_mulhi_pi16(srcG[i], G2V);
        dstV[i] = _mm_add_pi16(dstV[i], tmp);
        tmp = _mm_mulhi_pi16(srcB[i], B2V);
        dstV[i] = _mm_add_pi16(dstV[i], tmp);
        dstV[i] = _mm_add_pi16(dstV[i], UVConst128);
        
    }// get one frame - STEP B

    _mm_empty();
    
}


int process_with_mmx(YUV &OUT_YUV, const YUV &DEM1_YUV, const YUV &DEM2_YUV, RGB &CHECK_RGB1, RGB &CHECK_RGB2, const bool mode){
    clock_t begin_time = clock();
    clock_t total_time = 0;

    RGB rgb_blending = RGB(DEM1_YUV.width, DEM1_YUV.height);
    
    cout << "\nMMX..." << endl;
    clock_t core_time = clock();
    
    
    yuv2rgb_with_mmx(DEM1_YUV, CHECK_RGB1);
    if(mode)    yuv2rgb_with_mmx(DEM2_YUV, CHECK_RGB2);
    
    CHECK_RGB1.write(foutcheck2);
    
    
    for (int A = 1; A < 256; A += 3) {
        //blending_without_simd(rgb_blending, CHECK_RGB1, CHECK_RGB2, A, mode);
        blending_with_mmx(rgb_blending, CHECK_RGB1, CHECK_RGB2, A, mode);
        rgb2yuv_with_mmx(OUT_YUV, rgb_blending);
        
        total_time += clock() - core_time;
        core_time = clock();
        
        if(mode)
            OUT_YUV.write(fout12);
        else
            OUT_YUV.write(fout11);
    }// process end
    if(mode)    fclose(fout12);
    else    fclose(fout11);
    
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


