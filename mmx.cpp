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
    
    // YUV2RGB
    // const int16_t YUV2RGB_OFFSET[3][3] = {{1, 0, 1},{1, 0, 0},{1, 2, 0}};
    const __m64 UVConst128 = _mm_set_pi16(128, 128, 128, 128);
    const int16_t YUV2R[3] = {0,                         0, (int16_t)(0.140*(1<<16))};
    const int16_t YUV2G[3] = {0, (int16_t)(-0.394*(1<<16)), (int16_t)(-0.518*(1<<16))};
    const int16_t YUV2B[3] = {0,  (int16_t)(0.032*(1<<16)),                         0};
    const __m64 V2R = _mm_set_pi16(YUV2R[2], YUV2R[2], YUV2R[2], YUV2R[2]);       // 0.140
    const __m64 U2G = _mm_set_pi16(YUV2G[1], YUV2G[1], YUV2G[1], YUV2G[1]);       //-0.394
    const __m64 V2G = _mm_set_pi16(YUV2G[2], YUV2G[2], YUV2G[2], YUV2G[2]);       //-0.518
    const __m64 U2B = _mm_set_pi16(YUV2B[1], YUV2B[1], YUV2B[1], YUV2B[1]);       // 0.032
    
    // RGB2YUV
    const int16_t RGB2Y[3] = {(int16_t)(0.299*(1<<16)), (int16_t)(0.587*(1<<16)), (int16_t)(0.114*(1<<16))};
    const int16_t RGB2U[3] = {(int16_t)(-0.147*(1<<16)), (int16_t)(-0.289*(1<<16)), (int16_t)(0.436*(1<<16))};
    const int16_t RGB2V[3] = {(int16_t)(0.615*(1<<16)), (int16_t)(-0.515*(1<<16)), (int16_t)(-0.100*(1<<16))};
    const __m64 R2Y = _mm_set_pi16(RGB2Y[0], RGB2Y[0], RGB2Y[0], RGB2Y[0]);         // 0.299
    const __m64 G2Y = _mm_set_pi16(RGB2Y[1], RGB2Y[1], RGB2Y[1], RGB2Y[1]);         // 0.587
    const __m64 B2Y = _mm_set_pi16(RGB2Y[2], RGB2Y[2], RGB2Y[2], RGB2Y[2]);         // 0.114
    const __m64 R2U = _mm_set_pi16(RGB2U[0], RGB2U[0], RGB2U[0], RGB2U[0]);         //-0.147
    const __m64 G2U = _mm_set_pi16(RGB2U[1], RGB2U[1], RGB2U[1], RGB2U[1]);         //-0.289
    const __m64 B2U = _mm_set_pi16(RGB2U[2], RGB2U[2], RGB2U[2], RGB2U[2]);         // 0.436
    const __m64 R2V = _mm_set_pi16(RGB2V[0], RGB2V[0], RGB2V[0], RGB2V[0]);         // 0.615
    const __m64 G2V = _mm_set_pi16(RGB2V[1], RGB2V[1], RGB2V[1], RGB2V[1]);         //-0.515
    const __m64 B2V = _mm_set_pi16(RGB2V[2], RGB2V[2], RGB2V[2], RGB2V[2]);         //-0.100

    
    // duplicate U and V, preprocess for pack computing
    int16_t *dupPU16_1 = new int16_t[DEM1_YUV.size];
    int16_t *dupPV16_1 = new int16_t[DEM1_YUV.size];
    for(int i = 0; i < DEM1_YUV.size; i++){
        int iUV = DEM1_YUV.getBlockID(i);
        dupPU16_1[i] = DEM1_YUV.pU16[iUV];
        dupPV16_1[i] = DEM1_YUV.pV16[iUV];
    }
    
    int16_t *dupPU16_2 = new int16_t[DEM1_YUV.size];
    int16_t *dupPV16_2 = new int16_t[DEM1_YUV.size];
    for(int i = 0; i < DEM2_YUV.size; i++){
        int iUV = DEM2_YUV.getBlockID(i);
        dupPU16_2[i] = DEM2_YUV.pU16[iUV];
        dupPV16_2[i] = DEM2_YUV.pV16[iUV];
    }

    
    int64_t nloop = (DEM1_YUV.size >> 2);
    __m64 * Yp16_1 = (__m64 *)DEM1_YUV.pY16;
    __m64 * Up16_1 = (__m64 *)dupPU16_1;
    __m64 * Vp16_1 = (__m64 *)dupPV16_1;
    __m64 * Yp16_2 = (__m64 *)DEM2_YUV.pY16;
    __m64 * Up16_2 = (__m64 *)dupPU16_2;
    __m64 * Vp16_2 = (__m64 *)dupPV16_2;
    __m64 * dstR = (__m64*) CHECK_RGB.pB16;
    __m64 * dstG = (__m64*) CHECK_RGB.pG16;
    __m64 * dstB = (__m64*) CHECK_RGB.pB16;
    __m64 * dstY = (__m64*) OUT_YUV.pY16;
    __m64 * dstU = (__m64*) OUT_YUV.pU16;
    __m64 * dstV = (__m64*) OUT_YUV.pV16;
    int16_t * supR = new int16_t[nloop];
    int16_t * supG = new int16_t[nloop];
    int16_t * supB = new int16_t[nloop];
    
    __m64 tmp, tmpU, tmpV;
    
    _mm_empty();
    
    //const int16_t YUV2RGB_OFFSET[3][3] = {{1, 0, 1},{1, 0, 0},{1, 2, 0}};
    // V->R, U,V->G, U->B
    for (int A = 1; A < 256; A += 3) {
        clock_t core_time = clock();
        __m64 *RR = (__m64 *)CHECK_RGB.pR16;
        __m64 *GG = (__m64 *)CHECK_RGB.pG16;
        __m64 *BB = (__m64 *)CHECK_RGB.pB16;
        __m64 alpha64 = _mm_set_pi16((int16_t)A, (int16_t)A, (int16_t)A, (int16_t)A);
        for(int i = 0; i < nloop; i++){
            /*************** YUV2RGB ****************/
            tmp = _mm_setzero_si64();
            dstR[i] = _mm_setzero_si64();
            dstG[i] = _mm_setzero_si64();
            dstB[i] = _mm_setzero_si64();
            
            tmp = _mm_adds_pi16(tmp, Yp16_1[i]);
            dstR[i] = _mm_add_pi16(dstR[i], tmp);       // R = Y + ...
            dstG[i] = _mm_add_pi16(dstG[i], tmp);       // G = Y + ...
            dstB[i] = _mm_add_pi16(dstB[i], tmp);       // B = Y + ...
            
            // Get R
            tmpV = _mm_sub_pi16(Vp16_1[i], UVConst128);     // (V-128)
            dstR[i] = _mm_add_pi16(dstR[i], tmpV);          // R += (V-128)
            tmp = _mm_mulhi_pi16(tmpV, V2R);                // 0.140*(V-128)
            dstR[i] = _mm_add_pi16(dstR[i], tmp);           // R += 0.140*(V-128)
            
            // Get G
            tmpU = _mm_sub_pi16(Up16_1[i], UVConst128);
            tmp = _mm_mulhi_pi16(tmpU, U2G);
            dstG[i] = _mm_add_pi16(dstG[i], tmp);
            tmp = _mm_mulhi_pi16(tmpV, V2G);
            dstG[i] = _mm_add_pi16(dstG[i], tmp);
            
            // Get B
            tmp = _mm_mulhi_pi16(tmpU, U2B);
            dstB[i] = _mm_add_pi16(dstB[i], tmp);
            tmp = _mm_slli_si64(tmpU, 1);
            dstB[i] = _mm_add_pi16(dstB[i], tmp);
            
            
            /*************** Blending ****************/
            // 256 = 1 << 8
            RR[i] = _mm_mullo_pi16(dstR[i], alpha64);
            RR[i] = _mm_srli_pi16(RR[i], 8);
            GG[i] = _mm_mullo_pi16(dstG[i], alpha64);
            GG[i] = _mm_srli_pi16(GG[i], 8);
            BB[i] = _mm_mullo_pi16(dstB[i], alpha64);
            BB[i] = _mm_srli_pi16(BB[i], 8);
            
            /*************** RGB2YUV ****************/
            // Get Y
            dstY[i] = _mm_setzero_si64();
            tmp = _mm_mulhi_pi16(RR[i], R2Y);                  // 0.299*RR + 0.587*GG + 0.114*BB
            dstY[i] = _mm_add_pi16(dstY[i], tmp);
            tmp = _mm_mulhi_pi16(GG[i], G2Y);
            dstY[i] = _mm_add_pi16(dstY[i], tmp);
            tmp = _mm_mulhi_pi16(BB[i], B2Y);
            dstY[i] = _mm_add_pi16(dstY[i], tmp);
        }// get one picture - STEP A
        
        // Get U/V
        // Supress RGB to match U/V
        for(int row = 0, iUV = 0; row < DEM1_YUV.height; row += 2){
            for(int col = 0; col < DEM1_YUV.width; col += 2, iUV++){
                int iY = row * DEM1_YUV.width + col;
                supR[iUV] = CHECK_RGB.pR16[iY];
                supG[iUV] = CHECK_RGB.pG16[iY];
                supB[iUV] = CHECK_RGB.pB16[iY];
            }
        }
        // Compute U/V
        __m64 * srcR = (__m64*) supR;
        __m64 * srcG = (__m64*) supG;
        __m64 * srcB = (__m64*) supB;
        int64_t nloopUV = DEM1_YUV.size >> 4;
        for(int i = 0; i < nloopUV; i++){
            dstU[i] = _mm_setzero_si64();
            dstV[i] = _mm_setzero_si64();
            
            tmp = _mm_mulhi_pi16(srcR[i], R2U);              // -0.147*RR - 0.289*GG + 0.436*BB + 128;
            dstU[i] = _mm_add_pi16(dstU[i], tmp);
            tmp = _mm_mulhi_pi16(srcG[i], G2U);
            dstU[i] = _mm_add_pi16(dstU[i], tmp);
            tmp = _mm_mulhi_pi16(srcB[i], B2U);
            dstU[i] = _mm_add_pi16(dstU[i], tmp);
            dstU[i] = _mm_add_pi16(dstU[i], UVConst128);
            
            tmp = _mm_mulhi_pi16(srcR[i], R2V);             // 0.615*RR - 0.515*GG - 0.100*BB + 128;
            dstV[i] = _mm_add_pi16(dstV[i], tmp);
            tmp = _mm_mulhi_pi16(srcG[i], G2V);
            dstV[i] = _mm_add_pi16(dstV[i], tmp);
            tmp = _mm_mulhi_pi16(srcB[i], B2V);
            dstV[i] = _mm_add_pi16(dstV[i], tmp);
            dstV[i] = _mm_add_pi16(dstV[i], UVConst128);
            
        }// get one picture - STEP B
        
        total_time += clock() - core_time;
        if(A == 253)
            CHECK_RGB.write(foutcheck);
        OUT_YUV.write(fout11);
    }// process end
    fclose(fout11);
    fclose(foutcheck);

    
    cout << "Alpha Blending with dem1.yuv, output file is \"alpha1-1.yuv\":" <<endl;
    cout << "Core function time: " << (double)total_time / CLOCKS_PER_SEC * 1000<< "ms" << endl;
    cout << "Total run time: " << (double)(clock() - begin_time) / CLOCKS_PER_SEC * 1000 << "ms" << endl << endl;
    
    
    
    
    
    
 /*
    cout << "Alpha Blending with dem1.yuv and dem2.yuv, output file is \"alpha0-2.yuv\":" <<endl;
    cout << "Core function time: " << (double)total_time / CLOCKS_PER_SEC * 1000<< "ms" << endl;
    cout << "Total run time: " << (double)(clock() - begin_time) / CLOCKS_PER_SEC * 1000 << "ms" << endl << endl;

   */
    
    _mm_empty();
    
    return 0;
}
