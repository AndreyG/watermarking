#include "../stdafx.h"

#include "../geometry/planar_graph.h"

typedef geometry::planar_graph_t graph_t;

bool loops_exist(graph_t const & graph)
{
    for ( auto edge = graph.edges_begin(); edge != graph.edges_end(); ++edge )
    {
		if ( edge->b == edge->e )
            return true;
    }
    return false;
}

bool has_duplicate_vertices(graph_t const & graph)
{
    std::set< geometry::point_t > vertices( graph.vertices_begin(), graph.vertices_end() );
    return vertices.size() < graph.vertices_num();
}
