CC = g++

LIBS = -lCGAL -lGL -lGLU -lglut

all : compile
    
clean : 
	rm -f main

run : main
	main

compile : src/main.cpp
	$(CC) -o viewer $< $(LIBS)
