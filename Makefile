CC = g++

LIBS = -lboost_program_options-mt -lCGAL -lGL -lGLU -lglut  

all : compile
    
clean : 
	rm -f viewer

compile : src/main.cpp
	$(CC) -Wall -O3 -I /usr/local/include/lapackpp `pkg-config lapackpp --libs` -o viewer $< $(LIBS) 
