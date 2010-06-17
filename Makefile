CC = g++

LIBS = -lCGAL -lGL -lGLU -lglut -llapackpp

all : compile
    
clean : 
	rm -f main

run : viewer
	./viewer input/formatted-test.txt no

compile : src/main.cpp
	$(CC) -O3 -I /usr/local/include/lapackpp -o viewer $< $(LIBS)
