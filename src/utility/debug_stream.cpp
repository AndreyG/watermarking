#include "../stdafx.h"
#include "debug_stream.h"

namespace util
{
	void debug_stream::inc_indent() 
	{ 
		++tabs_num; 
		is_first_.top() = false; 
		is_first_.push( true );
	}

	void debug_stream::dec_indent() 
	{ 
		--tabs_num;
		is_first_.pop();
	}
	
	bool debug_stream::is_first() 
	{ 
		return is_first_.top();
	}

	debug_stream::~debug_stream()
	{
	    for ( size_t i = 0; i != tabs_num; ++i )
            std::cout << "|   ";
		std::cout << ss_.str() << std::endl;
	}

	size_t debug_stream::tabs_num = 0;

	namespace 
	{
		std::stack< bool > almost_empty_stack()
		{
			std::stack< bool > res;
			res.push( true );
			return res;
		}
	}
	
	std::stack< bool > debug_stream::is_first_ = almost_empty_stack();
}
