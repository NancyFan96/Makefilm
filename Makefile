CC = g++
PROC = makefilm

OBJECTS = main.o yuv.o rgb.o

MAIN= main.o yuv.o rgb.o
YUV= yuv.o
RGB = rgb.o

SRC=	main.cpp yuv.cpp rgb.cpp
HDR=	yuv.hpp rgb.hpp bmp.h

makefilm: 	$(SRC) $(HDR)
	$(CC) -o $(PROC) *.cpp  -Wall

$(YUV): yuv.hpp
	
$(RGB): rgb.hpp bmp.h

$(MAIN): $(HDR)


.PHONY : clean
clean:
	-rm $(PROC) 
