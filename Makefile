CC = g++
PROC = makefilm

SRC=	main.cpp yuv.cpp rgb.cpp
HDR=	yuv.hpp rgb.hpp bmp.h


all: 	$(PROC)

sim: 	$(SRC) $(HDR)
	$(CC) -o $(PROC) *.cpp *.cc -Wall


clean:
	-rm $(PROC) 
