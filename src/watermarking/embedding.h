#ifndef _EMBEDDING_H_
#define _EMBEDDING_H_

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Constrained_Delaunay_triangulation_2.h>

#include "spectral_analysis.h"
#include "../geometry/plane_subdivision.h"
#include "../geometry/triangulation_graph.h"

namespace watermarking
{
    const size_t MAX_PATCH_SIZE = 1000;

    template< class Point >
    struct planar_graph
    {
        typedef Point                       vertex_t;
        typedef std::vector< vertex_t >     vertices_t;
        typedef std::pair< size_t, size_t > edge_t;
        typedef std::vector< edge_t >       edges_t;

        vertices_t   vertices;
        edges_t     edges;
    };

    typedef std::vector< int > message_t;

    template<class Point>
    struct embedding_impl
    {
        typedef planar_graph< Point > graph_t;

        embedding_impl( graph_t const & graph, message_t const & message )
                : graph_( graph )
        {
            size_t subareas_num = subdivide_plane( MAX_PATCH_SIZE );
            build_trgs( subareas_num );
            foreach ( CDT const & trg, trgs_ )
            {
                vertices_t r = coefficients( trg );
            }
        }

    private:
        typedef CGAL::Exact_predicates_inexact_constructions_kernel     Kernel;
        typedef CGAL::Constrained_Delaunay_triangulation_2< Kernel >    CDT;
        typedef typename graph_t::vertices_t                            vertices_t;

        size_t subdivide_plane( size_t max_subarea_size )
        {
            std::map< Point, size_t > old_index;
            size_t i = 0;
            foreach ( typename graph_t::vertex_t const & v, graph_.vertices )
            {
                old_index.insert( std::make_pair( v, i ) );
                ++i;
            }
            size_t res = geometry::subdivide_plane( graph_.vertices.begin(), graph_.vertices.end(), max_subarea_size, true, subdivision_, 0 );
            index_.resize( graph_.vertices.size() );
            for ( i = 0; i != index_.size(); ++i )
            {
                index_[old_index[graph_.vertices[i]]] = i;
            }            
            return res;
        }

        void build_trgs( size_t subareas_num )
        {
            trgs_.resize( subareas_num );
            foreach ( typename graph_t::edge_t const & edge, graph_.edges )
            {
                size_t begin = index_[edge.first], end = index_[edge.second];
                if ( subdivision_[begin] == index_[end] )
                {
                    trgs_[subdivision_[begin]].insert_constraint( graph_.vertices[begin], graph_.vertices[end] );
                }
            }
        }        
                
        vertices_t coefficients( CDT const & trg ) const
        {
            vertices_t vertices( trg.number_of_vertices() );
            std::map< CDT::Vertex_handle, size_t > trg_vertex_to_index;
            size_t i = 0;
            typedef CDT::Finite_vertices_iterator vertices_iterator;
            for ( vertices_iterator v = trg.vertices_begin(); v != trg.vertices_end(); ++v )
            {
                trg_vertex_to_index.insert( std::make_pair( v, i ) );
                vertices.push_back( v->point() );
                ++i;
            }
            typedef geometry::triangulation_graph< CDT > incidence_graph;
            return  spectral_coefficients( incidence_graph( trg, trg_vertex_to_index ), vertices );
        }

        std::vector< size_t >   index_;
        graph_t                 graph_;
        std::vector< CDT >      trgs_;
        std::vector< size_t >   subdivision_;
    };
    
    template< class Point >
    std::auto_ptr< embedding_impl< Point > > embed( planar_graph< Point > const & graph, message_t const & message )
    {
        return std::auto_ptr< embedding_impl< Point > >( new embedding_impl< Point >( graph, message ) );
    }
}

#endif /* _EMBEDDING_H_ */
