CC = g++

LIBS = -lCGAL

all : compile
    
clean : 
	rm -f main

run : main
	main

compile : src/main.cpp
	$(CC) -o main $< $(LIBS)
