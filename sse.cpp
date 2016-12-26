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

using namespace std;

int process_with_sse(YUV &OUT_YUV, YUV &DEM1_YUV, YUV &DEM2_YUV, RGB &CHECK_RGB, RGB &CHECK_RGB2){
    clock_t begin_time, total_time;
    begin_time = clock();
    total_time = 0;
    
    cout << "\nSSE..." << endl;

    return 0;
}
