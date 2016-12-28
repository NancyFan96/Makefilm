# Accelerate IMG Process via SIMD

## LOGs

* 1.0 commit: Finish no simd  version

* 2.0 commit: **REALIZE IT**!

## TOD

* MMX
* SSE
* AVX

## QuikStart


This is Lab4.1 of Computer Architecture. I call it "makefilm", mainly because you CAN:

* Fade in and fade out one image.
* Create a gradient which overlays two images.


### Help info

```
    Use dem1.yuv, dem1.yuv to process...
    Usage: ./makefilm <mode>
           mode [default]: mode 1 and mode 2
```


## Files included

```
├──	readme.md   
├──	Makefile  
├──	system.h  
├── main.cpp  
├── nosimd.cpp					# core codes
├── mmx.cpp  					# ...
├── sse.cpp 					# ...
├── avx.cpp 					# ...
├── yuv.cpp  					# definition of class YUV
├── yuv.hpp   					# ...
├── rgb.cpp  					# definition of class RGB, useful to debug
├── rgb.hpp   					# ...
├── bmp.h   					# Useful to output BMP pics to debug
├── input/ 							  
│   ├── dem1.yuv 					
│   ├── dem1.yuv					
│── output/  
│   ├── alpha0-1.yuv  			# result file without simd (One pic)
│   ├── alpha0-2.yuv  			# result file without simd (Two pics)
│   ├── alpha1-1.yuv  			# result files via MMX
│   ├── alpha1-2.yuv  			# ...
│   ├── alpha2-1.yuv  			# result files via sse
│   ├── alpha2-2.yuv  			# ...
│   ├── alpha3-1.yuv  			# result files via avx
│   ├── alpha3-2.yuv  			# ...

```

