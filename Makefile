CC = g++

LIBS = -lCGAL -lGL -lGLU -lglut

all : compile
    
clean : 
	rm -f main

run : viewer
	./viewer input/test.txt

compile : src/main.cpp
	$(CC) -O3 -o viewer $< $(LIBS)
