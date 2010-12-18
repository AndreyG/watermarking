#include "../stdafx.h"
#include "stopwatch.h"

namespace util
{
    stopwatch::stopwatch( const char * message )
    {
        for ( size_t i = 0; i != tabs_num; ++i )
            std::cout << "\t";
        std::cout << message << "..." << std::endl;
        ++tabs_num;
        start = clock();
    }

    stopwatch::~stopwatch()
    {
        --tabs_num;
        for ( size_t i = 0; i != tabs_num; ++i )
            std::cout << "\t";
        clock_t end = clock();
        std::cout << double(end - start) / CLOCKS_PER_SEC << " sec" << std::endl;
    }

	size_t stopwatch::tabs_num = 0;
}

