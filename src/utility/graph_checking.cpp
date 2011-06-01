#include "../stdafx.h"

#include "../utility/debug_stream.h"
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
    if (vertices.size() < graph.vertices_num())
    {
        util::debug_stream(util::debug_stream::WARNING) << "vertices num:          " << graph.vertices_num();
        util::debug_stream(util::debug_stream::WARNING) << "unique vertices num:   " << vertices.size();
    }
    return vertices.size() < graph.vertices_num();
}
