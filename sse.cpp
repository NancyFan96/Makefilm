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
extern FILE *foutcheck1, foutcheck2;

extern void yuv2rgb_without_simd(const YUV & yuv, RGB & rgb);
extern void blending_without_simd(RGB & rgb_blending, const RGB & rgb1, const RGB & rgb2, const int A, const bool mode);
extern void rgb2yuv_without_simd(YUV & yuv,const RGB & rgb);


void yuv2rgb_with_sse(const YUV & yuv, RGB & rgb){
    
}
void blending_with_sse(RGB & rgb_blending, const RGB & rgb1, const RGB & rgb2, const int A, const bool mode){
    
}
void rgb2yuv_with_sse(YUV & yuv,const RGB & rgb){
    
}


int process_with_sse(YUV &OUT_YUV, const YUV &DEM1_YUV, const YUV &DEM2_YUV, RGB &CHECK_RGB1, RGB &CHECK_RGB2, const bool mode){
    clock_t begin_time = clock();
    clock_t total_time = 0;
    
    RGB rgb_blending = RGB(DEM1_YUV.width, DEM1_YUV.height);
    
    cout << "\nSSE..." << endl;
    clock_t core_time = clock();
    
    _mm_empty();
    yuv2rgb_with_sse(DEM1_YUV, CHECK_RGB1);
    if(mode)    yuv2rgb_with_sse(DEM2_YUV, CHECK_RGB2);
    
    for (int A = 1; A < 256; A += 3) {
        blending_with_sse(rgb_blending, CHECK_RGB1, CHECK_RGB2, A, mode);
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
    
    _mm_empty();
    return 0;
}
