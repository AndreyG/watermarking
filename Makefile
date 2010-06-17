CC = g++

LIBS = -lboost_program_options -lCGAL -lGL -lGLU -lglut -llapackpp

all : compile
    
clean : 
	rm -f main

compile : src/main.cpp
	$(CC) -O3 -I /usr/local/include/lapackpp -o viewer $< $(LIBS)
