CC = g++

LIBS = -lboost_program_options-mt -lCGAL -lGL -lGLU -lglut  

all : precompile compile
    
clean : 
	rm -f viewer

compile : src/main.cpp
	$(CC) -std=c++0x -Wall -O3 -I /usr/local/include/lapackpp -o viewer $< $(LIBS) 

precompile : src/stdafx.h  
	$(CC) -c src/stdafx.h -o src/stdafx.h.gch
