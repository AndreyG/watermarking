CC = g++

LIBS = -lboost_program_options-mt -lCGAL -lGL -lGLU -lglut -llapackpp 

all : compile-main compile-white-noise
    
clean : 
	rm -f viewer

compile-main : src/main.cpp
    $(CC) -std=c++0x -Wall -O3 -I /usr/local/include/lapackpp -o viewer $< $(LIBS) 
compile-white-noise: src/experiments/white-noise.cpp
	$(CC) -std=c++0x -Wall -O3 -I /usr/local/include/lapackpp -o white-noise $< $(LIBS) 
