#ifndef _INOUT_H_
#define _INOUT_H_

#include "../geometry/planar_graph.h"

namespace inout
{
	void read_graph( geometry::planar_graph_t & graph, std::istream & in );

	struct message_params_t
	{
    	std::string text;
    	size_t      chip_rate;
    	int         key;
    	double      alpha;
	};

	message_params_t read_message_params(const char * filepath);
	
	template< class Stream >
	void skip_char( Stream & in, char barrier )
	{
		while ( in )
		{
			char c;
			in >> c;
			if ( c == barrier )
				break;
		}
	}

	void write_graph( geometry::planar_graph_t const & graph, std::ostream & out );
}

#endif /*_INOUT_H_*/
