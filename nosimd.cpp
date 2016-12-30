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
extern FILE *foutcheck1, *foutcheck2;

/* 
 * YUV2RGB
 * R = 1.164383 * (Y - 16) + 1.596027*(V - 128)
 * G = 1.164383 * (Y - 16) – 0.391762*(U - 128) – 0.812968*(V - 128)
 * B = 1.164383 * (Y - 16) + 2.017232*(U - 128)
 */
void yuv2rgb_without_simd(const YUV& yuv, RGB& rgb) {
    yuv.u8_to_s16();
	for (int iY = 0; iY < yuv.size; iY++) {
		int iUV = yuv.getBlockID(iY);
		rgb.pR16[iY] = 1.164383 * (yuv.pY16[iY] - 16) + (1.596027 * (yuv.pV16[iUV] - 128));
		rgb.pG16[iY] = 1.164383 * (yuv.pY16[iY] - 16) - 0.391762 * (yuv.pU16[iUV] - 128) - 0.812968 * (yuv.pV16[iUV] - 128);
		rgb.pB16[iY] = 1.164383 * (yuv.pY16[iY] - 16) + (2.017232 * (yuv.pU16[iUV] - 128));
	}
	rgb.round();
}

/*
 * Blending 
 */
void blending_without_simd(RGB& rgb_blending, const RGB& rgb1, const RGB& rgb2, const int A, const bool mode) {
	if (mode) {
		int _A = 256 - A;
		for (int iY = 0; iY < rgb1.size; iY++) {
			rgb_blending.pR16[iY] = (A * rgb1.pR16[iY] + _A * rgb2.pR16[iY]) >> 8;
		}
		for (int iY = 0; iY < rgb1.size; iY++) {
			rgb_blending.pG16[iY] = (A * rgb1.pG16[iY] + _A * rgb2.pG16[iY]) >> 8;
		}
		for (int iY = 0; iY < rgb1.size; iY++) {
			rgb_blending.pB16[iY] = (A * rgb1.pB16[iY] + _A * rgb2.pB16[iY]) >> 8;
		}
	}
	else {
		for (int iY = 0; iY < rgb1.size; iY++) {
			rgb_blending.pR16[iY] = ((uint16_t)A * rgb1.pR16[iY]) >> 8;
		}
		for (int iY = 0; iY < rgb1.size; iY++) {
			rgb_blending.pG16[iY] = ((uint16_t)A * rgb1.pG16[iY]) >> 8;
		}
		for (int iY = 0; iY < rgb1.size; iY++) {
			rgb_blending.pB16[iY] = ((uint16_t)A * rgb1.pB16[iY]) >> 8;
		}
	}
}

/* 
 * RGB2YUV
 * Y= 0.256788*R + 0.504129*G + 0.097906*B + 16
 * U= -0.148223*R - 0.290993*G + 0.439216*B + 128
 * V= 0.439216*R - 0.367788*G - 0.071427*B + 128
 */
void rgb2yuv_without_simd(YUV& yuv, const RGB& rgb) {
	for (int row = 0, iY = 0, iUV = 0; row < yuv.height; row++) {
		for (int col = 0; col < yuv.width; col++ , iY++) {
			yuv.pY16[iY] = 0.256788 * rgb.pR16[iY] + 0.504129 * rgb.pG16[iY] + 0.097906 * rgb.pB16[iY] + 16;

			if ((row & 1) && (col & 1)) {
				yuv.pU16[iUV] = -0.148223 * rgb.pR16[iY] - 0.2909931 * rgb.pG16[iY] + 0.439216 * rgb.pB16[iY] + 128;
				yuv.pV16[iUV] = 0.439216 * rgb.pR16[iY] - 0.367788 * rgb.pG16[iY] - 0.071427 * rgb.pB16[iY] + 128;
				iUV++;
			}
		}
	}// get one picture
    yuv.s16_to_u8();
}

int process_without_simd(YUV& OUT_YUV, const YUV& DEM1_YUV, const YUV& DEM2_YUV, RGB& CHECK_RGB1, RGB& CHECK_RGB2, const bool mode) {
	clock_t begin_time = clock();
	clock_t total_time = 0;

	RGB rgb_blending = RGB(DEM1_YUV.width, DEM1_YUV.height);

	cout << "\nNO SIMD..." << endl;
	clock_t core_time = clock();

	yuv2rgb_without_simd(DEM1_YUV, CHECK_RGB1);
	if (mode)
		yuv2rgb_without_simd(DEM2_YUV, CHECK_RGB2);

    
	for (int A = 1; A < 256; A += 3) {
		blending_without_simd(rgb_blending, CHECK_RGB1, CHECK_RGB2, A, mode);
		rgb2yuv_without_simd(OUT_YUV, rgb_blending);

		total_time += clock() - core_time;
		core_time = clock();

		if (mode)
			OUT_YUV.write(fout02);
		else
			OUT_YUV.write(fout01);
	}// process end
	if (mode) fclose(fout02);
	else fclose(fout01);

	if (mode) {
		cout << "Alpha Blending with dem1.yuv and dem2.yuv, output file is \"alpha0-2.yuv\":" << endl;
		cout << "Core function time: " << (double)total_time / CLOCKS_PER_SEC * 1000 << "ms" << endl;
		cout << "Include output time: " << (double)(clock() - begin_time) / CLOCKS_PER_SEC * 1000 << "ms" << endl << endl;
	}
	else {
		cout << "Alpha Blending with dem1.yuv, output file is \"alpha0-1.yuv\":" << endl;
		cout << "Core function time: " << (double)total_time / CLOCKS_PER_SEC * 1000 << "ms" << endl;
		cout << "Include output time: " << (double)(clock() - begin_time) / CLOCKS_PER_SEC * 1000 << "ms" << endl << endl;
	}


	return 0;
}
