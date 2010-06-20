#ifndef _WATERMARKING_COMMON_H_
#define _WATERMARKING_COMMON_H_

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_2.h>
#include <CGAL/Constrained_Delaunay_triangulation_2.h>

#include "../utility/random_generator.h"
#include "../geometry/plane_subdivision.h"
#include "spectral_analysis.h"
#include "../geometry/triangulation_graph.h"

namespace watermarking
{
    template< class Point >
    struct planar_graph
    {
        typedef Point                       vertex_t;
        typedef std::vector< vertex_t >     vertices_t;
        typedef std::pair< size_t, size_t > edge_t;
        typedef std::vector< edge_t >       edges_t;

        vertices_t   vertices;
        edges_t      edges;
    };

    typedef std::vector< int > message_t;

    template< class Point >
    planar_graph< Point > add_noise( planar_graph< Point > g, double radius )
    {
        typedef typename planar_graph< Point >::vertex_t vertex_t;

        util::random_generator gen( radius );
        foreach( vertex_t & v, g.vertices )
        {
            double a = ( 2 * M_PI * rand() ) / RAND_MAX;
            double r = gen();
            v = vertex_t( v.x() + cos( a ) * r, v.y() + sin( a ) * r );
        }
        return g;
    }

    template< class Graph >
    size_t subdivide_plane( Graph & graph, size_t max_subarea_size, std::vector< size_t > & subdivision )
    {
        util::stopwatch _( "subidivide plane" );

        typedef typename Graph::vertex_t vertex_t;
        std::map< vertex_t, size_t > old_index;
        size_t i = 0;
        foreach ( vertex_t const & v, graph.vertices )
        {
            old_index.insert( std::make_pair( v, i ) );
            ++i;
        }

        size_t res = geometry::subdivide_plane( graph.vertices.begin(), graph.vertices.end(), max_subarea_size, true, subdivision, 0 );

        std::vector< size_t > old2new( graph.vertices.size() );
        for ( size_t i = 0; i != old2new.size(); ++i )
        {
            old2new[old_index[graph.vertices[i]]] = i;
        }

        typedef typename Graph::edge_t edge_t; 
        foreach( edge_t & e, graph.edges )
        {
            e.first = old2new[e.first];
            e.second = old2new[e.second];
        }
        return res;
    }

    typedef boost::shared_ptr< spectral_analyser > analyser_ptr;
  
    template< class Trg, class Vertex >
    std::vector< Vertex > coefficients( Trg const & trg, analyser_ptr & analyser, bool weighted )
    {
        std::vector< Vertex > vertices;
        
        std::map< typename Trg::Vertex_handle, size_t > trg_vertex_to_index;
        size_t i = 0;
        typedef typename Trg::Finite_vertices_iterator vertices_iterator;
        for ( vertices_iterator v = trg.finite_vertices_begin(); v != trg.finite_vertices_end(); ++v )
        {
            trg_vertex_to_index.insert( std::make_pair( v, i ) );
            vertices.push_back( v->point() );
            ++i;
        }

        typedef geometry::triangulation_graph< Trg > incidence_graph;
        analyser.reset( new spectral_analyser( incidence_graph( trg, trg_vertex_to_index, weighted ) ) );
        
        return analyser->get_coefficients( vertices );
    }
}
#endif /* _WATERMARKING_COMMON_H_ */
