MKLROOT=/opt/intel/mkl
MKLLIB=$(MKLROOT)/lib/ia32

compile: bin/main.o bin/embedding.o bin/stopwatch.o bin/data_reading.o bin/data_writing.o bin/graph_fixing.o
	g++ -m32 -w bin/main.o bin/embedding.o bin/stopwatch.o bin/data_reading.o bin/data_writing.o bin/graph_fixing.o \
	-Wl,--start-group \
    "$(MKLLIB)/libmkl_intel.a" \
    "$(MKLLIB)/libmkl_gnu_thread.a" \
    "$(MKLLIB)/libmkl_core.a" \
    -Wl,--end-group \
	-L"$(MKLROOT)/../compiler/lib/ia32" \
	-liomp5 -lpthread -lm -lCGAL -lboost_program_options-mt -o bin/watermarking

src/stdafx.h.gch: src/stdafx.h 
	g++ -std=c++0x -c -Wall src/stdafx.h -I"$(MKLROOT)/include" -o src/stdafx.h.gch

bin/main.o: src/main.cpp src/stdafx.h.gch
	g++ -std=c++0x -c -Wall src/main.cpp -o bin/main.o 

bin/embedding.o: src/watermarking/embedding.cpp src/stdafx.h.gch
	g++ -std=c++0x -c -Wall src/watermarking/embedding.cpp -o bin/embedding.o 

bin/stopwatch.o: src/utility/stopwatch.cpp src/stdafx.h.gch 
	g++ -std=c++0x -c -Wall src/utility/stopwatch.cpp -o bin/stopwatch.o

bin/data_reading.o: src/inout/data_reading.cpp src/stdafx.h.gch 
	g++ -std=c++0x -c -Wall src/inout/data_reading.cpp -o bin/data_reading.o

bin/data_writing.o: src/inout/data_writing.cpp src/stdafx.h.gch
	g++ -std=c++0x -c -Wall src/inout/data_writing.cpp -o bin/data_writing.o

bin/graph_fixing.o: src/utility/graph_fixing.cpp src/stdafx.h.gch
	g++ -std=c++0x -c -Wall src/utility/graph_fixing.cpp -o bin/graph_fixing.o
