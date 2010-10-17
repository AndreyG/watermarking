#pragma once

#include "geometry\primitives\point_fwd.h"
#include "geometry\point_io.h"

struct graph_t
{
	typedef cg::point_2					vertex_t;
	typedef std::pair< size_t, size_t > edge_t;
	
	std::vector< vertex_t >	vertices;
	std::vector< edge_t >	edges;

	template<class Stream>
	explicit graph_t(Stream & in);
};

size_t vertices_num(graph_t const & graph)
{
	return graph.vertices.size();
}

size_t edges_num(graph_t const & graph)
{
	return graph.edges.size();
}

template<class Stream>
graph_t::graph_t(Stream & in)
{
	int n, m;
	in >> n >> m;
	vertices.resize(n);
	edges.resize(m);
	for (int i = 0; i != n; ++i)
		in >> vertices[i];
	for (int i = 0; i != m; ++i)
		in >> edges[i].first >> edges[i].second;
}

void fix_graph(graph_t & g)
{
	typedef graph_t::vertex_t vertex_t;
    typedef graph_t::edge_t edge_t;
    typedef std::map< vertex_t, size_t > v2i_t;  

    vertex_t middle;
    for each ( vertex_t const & v in g.vertices )
    {
        middle += v;
    }
	middle /= vertices_num(g);
	for ( std::vector< vertex_t >::iterator v = g.vertices.begin(); v != g.vertices.end(); ++v )
    {
        *v -= middle;
    }

    v2i_t v2i;
    size_t idx = 0;
    for each ( vertex_t const & v in g.vertices )
    {
        v2i_t::iterator it = v2i.find( v );
        if ( it == v2i.end() )
        {    
            v2i.insert( it, std::make_pair( v, idx ) );
            ++idx;
        }
    }
	std::vector< vertex_t > new_vertices;
    std::set< vertex_t > vertex_set;
    for each ( vertex_t const & v in g.vertices )
    {
        if ( !vertex_set.count( v ) )
        {
            vertex_set.insert( v );
            new_vertices.push_back( v );
        }
    }

	std::vector< edge_t > new_edges;
    for each ( edge_t const & e in g.edges )
    {
        new_edges.push_back( edge_t( v2i[g.vertices[e.first]], v2i[g.vertices[e.second]] ) );
    }

    g.vertices.swap( new_vertices );
    g.edges.swap( new_edges );
}