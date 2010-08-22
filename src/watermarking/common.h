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
        foreach ( vertex_t & v, g.vertices )
        {
            double a = ( 2 * M_PI * rand() ) / RAND_MAX;
            double r = gen();
            v = vertex_t( v.x() + cos( a ) * r, v.y() + sin( a ) * r );
        }
        return g;
    }

    // vector subdivision will be filled by indices of zone in which graph.vertices[i] will be laid.
    template< class Graph >
    size_t subdivide_plane( Graph & graph, size_t max_subarea_size, std::vector< size_t > & subdivision )
    {
    }

    typedef boost::shared_ptr< spectral_analyser > analyser_ptr;
  
}
#endif /* _WATERMARKING_COMMON_H_ */
