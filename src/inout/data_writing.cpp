#include "../stdafx.h"

#include "inout.h"

namespace inout
{
	void write_graph( geometry::planar_graph_t const & graph, std::ostream & out )
	{
    	out << graph.vertices_num() << "\t" << graph.edges_num() << "\n";
		for ( auto v = graph.vertices_begin(); v != graph.vertices_end(); ++v )
    	{
			out << *v << "\n";
    	}
		for ( auto e = graph.edges_begin(); e != graph.edges_end(); ++e )
    	{
        	out << e->b << " " << e->e << "\n";
    	}
	}
}
