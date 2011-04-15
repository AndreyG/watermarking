#include "../stdafx.h"
#include "stopwatch.h"
#include "debug_stream.h"

namespace util
{
    stopwatch::stopwatch( const char * message ) { init(message); }
    stopwatch::stopwatch( std::string const & message ) { init(message.c_str()); }
    stopwatch::stopwatch( boost::format const & message ) { init(message.str().c_str()); }

    void stopwatch::init( const char * message )
    {
		if (!debug_stream::is_first())
			debug_stream() << "";
        debug_stream() << message << "...";
		debug_stream::inc_indent();
        start = clock();
    }

    stopwatch::~stopwatch()
    {
        clock_t end = clock();
		debug_stream::dec_indent();
        debug_stream() << double(end - start) / CLOCKS_PER_SEC << " sec";
    }
}
