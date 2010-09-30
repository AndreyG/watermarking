#!/bin/sh

libs='-lboost_program_options-mt -lCGAL -lGL -lGLU -lglut -llapackpp -lgmp'
options='-std=c++0x -Wall -O2 -frounding-math'
include='-I /usr/local/include/lapackpp'

if [ ! -d bin ]; then
    mkdir bin    
fi

g++ $options $include -o bin/watermarking src/main.cpp $libs
