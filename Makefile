CC = g++
PROC = makefilm

OPTIMIZE = -O2
WARNING = -Wno-unused-result
SETTINGS = -std=c++11 -msse2 -mmmx -mavx2
CFLAGS = $(OPTIMIZE) $(WARNING) $(SETTINGS)


OBJECTS = main.o yuv.o rgb.o

MAIN= main.o yuv.o rgb.o
YUV= yuv.o
RGB = rgb.o
NOSIMD = nosimd.o
MMX = mmx.o
SSE = sse.o
AVX = avx.o

SRC=	main.cpp yuv.cpp rgb.cpp nosimd.cpp mmx.cpp sse.cpp avx.cpp
HDR=	yuv.hpp rgb.hpp bmp.h system.h
CORE=   nosimd.cpp mmx.cpp sse.cpp avx.cpp

all: $(PROC)

makefilm: $(SRC) $(HDR)
	$(CC) -o $(PROC) $(SRC) $(CFLAGS)

$(MAIN) $(NOSIMD): $(HDR)

$(YUV): yuv.hpp
	
$(RGB): rgb.hpp bmp.h



.PHONY : clean
clean:
	-rm $(PROC) *.o
