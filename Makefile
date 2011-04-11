MKLLIB=/opt/intel/mkl/lib/intel64
CXXFLAGS=-std=c++0x -Wall -c -g3 -frounding-math

OBJ_FILES=	bin/main.o bin/embedding.o bin/extracting.o bin/stopwatch.o bin/data_reading.o \
			bin/data_writing.o bin/graph_fixing.o bin/graph_checking.o bin/debug_stream.o bin/statistics.o
 
compile: $(OBJ_FILES)
	g++ $(OBJ_FILES) \
	-Wl,--start-group \
	"$(MKLLIB)/libmkl_intel_lp64.a" \
    "$(MKLLIB)/libmkl_gnu_thread.a" \
    "$(MKLLIB)/libmkl_core.a" \
    -Wl,--end-group \
	-lgomp -lpthread -lgmp -lboost_program_options -lCGAL -o bin/watermarking

src/stdafx.h.gch: src/stdafx.h 
	g++ $(CXXFLAGS) src/stdafx.h -I"$(MKLROOT)/include" -o src/stdafx.h.gch


bin/main.o: src/main.cpp \
			src/watermarking/embedding.h \
			src/watermarking/extracting.h \
			src/stdafx.h.gch
	test -d bin || mkdir bin
	g++ $(CXXFLAGS) src/main.cpp -o bin/main.o 

bin/embedding.o: 	src/watermarking/embedding.cpp \
					src/watermakring/embedding.h \
					src/geometry/triangulation_graph.h \
					src/geometry/plane_subdivision.h \
					src/inout/inout.h \
					src/watermarking/weights/unweighted.h \
					src/watermarking/weights/conformal.h \
					src/watermarking/weights/dirichlet.h \
					src/stdafx.h.gch 
	g++ $(CXXFLAGS) src/watermarking/embedding.cpp -o bin/embedding.o 

src/watermakring/embedding.h: src/watermarking/common.h

src/watermarking/weights/unweighted.h: 	src/geometry/triangulation_graph.h \
										src/watermarking/spectral_analysis.h

src/watermarking/weights/conformal.h: 	src/geometry/triangulation_graph.h \
										src/geometry/trigonometry.h \
										src/watermarking/spectral_analysis.h

src/watermarking/weights/dirichlet.h: 	src/geometry/triangulation_graph.h \
										src/geometry/trigonometry.h \
										src/watermarking/spectral_analysis.h

bin/extracting.o: 	src/watermarking/extracting.cpp \
					src/watermakring/extracting.h \
					src/stdafx.h.gch
	g++ $(CXXFLAGS) src/watermarking/extracting.cpp -o bin/extracting.o 

src/watermakring/extracting.h: src/watermarking/common.h

src/watermarking/common.h: 	src/watermarking/spectral_analysis.h \
							src/geometry/planar_graph.h

bin/stopwatch.o: 	src/utility/stopwatch.cpp \
					src/stdafx.h.gch 
	g++ $(CXXFLAGS) src/utility/stopwatch.cpp -o bin/stopwatch.o

bin/debug_stream.o: src/utility/debug_stream.cpp \
					src/stdafx.h.gch 
	g++ $(CXXFLAGS) src/utility/debug_stream.cpp -o bin/debug_stream.o

bin/data_reading.o: src/inout/data_reading.cpp \
					src/stdafx.h.gch 
	g++ $(CXXFLAGS) src/inout/data_reading.cpp -o bin/data_reading.o

bin/data_writing.o: src/inout/data_writing.cpp \
					src/stdafx.h.gch
	g++ $(CXXFLAGS) src/inout/data_writing.cpp -o bin/data_writing.o

bin/graph_fixing.o: src/utility/graph_fixing.cpp \
					src/stdafx.h.gch
	g++ $(CXXFLAGS) src/utility/graph_fixing.cpp -o bin/graph_fixing.o

bin/graph_checking.o: 	src/utility/graph_checking.cpp \
						src/stdafx.h.gch
	g++ $(CXXFLAGS) src/utility/graph_checking.cpp -o bin/graph_checking.o

bin/statistics.o:	src/statistics.cpp \
					src/statistics.h \
					src/stdafx.h.gch
	g++ $(CXXFLAGS) src/statistics.cpp -o bin/statistics.o

clean:
	rm -f bin/*.o src/stdafx.h.gch
