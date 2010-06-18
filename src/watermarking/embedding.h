#ifndef _EMBEDDING_H_
#define _EMBEDDING_H_

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_2.h>
#include <CGAL/Constrained_Delaunay_triangulation_2.h>

#include <boost/lexical_cast.hpp>

using boost::lexical_cast;

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
        edges_t      edges;
    };

    typedef std::vector< int > message_t;

    template<class Point>
    struct embedding_impl
    {
        typedef planar_graph< Point > graph_t;
        typedef CGAL::Exact_predicates_inexact_constructions_kernel     Gt;
        typedef CGAL::Constrained_Delaunay_triangulation_2< Gt > trg_t;

        typedef typename graph_t::vertices_t vertices_t;

        enum step_t
        {
            SUBDIVIDE_PLANE, BUILD_TRIANGULATIONS, FACTORIZE, MODIFY_VERTICES, STEP_SIZE
        };
        
        embedding_impl( graph_t const & graph, bool weighted, bool use_edges, bool step_by_step )
                : graph_( graph )
                , weighted_( weighted )
                , use_edges_( use_edges )
        {
            step_ = SUBDIVIDE_PLANE;
            if ( !step_by_step )
            	while ( next_step() );
        }

        bool next_step()
        {
            switch ( step_ )
            {
            case SUBDIVIDE_PLANE:
                subareas_num_ = subdivide_plane( MAX_PATCH_SIZE );
                step_ = BUILD_TRIANGULATIONS;
                std::cout << "number of subareas " << subareas_num_ << std::endl;
                return true;
            case BUILD_TRIANGULATIONS:
                build_trgs( subareas_num_ );
                step_ = FACTORIZE;
                return true;
            case FACTORIZE:
                factorize( subareas_num_ );
                step_ = MODIFY_VERTICES;
                return true;
            default:
            	return false;
            }
        }

//    private:

        size_t subdivide_plane( size_t max_subarea_size )
        {
            util::stopwatch _( "subidivide plane" );
            std::map< Point, size_t > old_index;
            size_t i = 0;
            foreach ( typename graph_t::vertex_t const & v, graph_.vertices )
            {
                old_index.insert( std::make_pair( v, i ) );
                ++i;
            }

            size_t res = geometry::subdivide_plane( graph_.vertices.begin(), graph_.vertices.end(),
													max_subarea_size, true, subdivision_, 0 );
            std::vector< size_t > old2new( graph_.vertices.size() );
            for ( size_t i = 0; i != old2new.size(); ++i )
            {
                old2new[old_index[graph_.vertices[i]]] = i;
            }
            typedef typename graph_t::edge_t edge_t; 
            foreach( edge_t & e, graph_.edges )
            {
                e.first = old2new[e.first];
                e.second = old2new[e.second];
            }
            return res;
        }

        void build_trgs( size_t subareas_num )
        {
            util::stopwatch _( "build_triangulations" );
            trgs_.resize( subareas_num );

            for ( size_t i = 0; i != graph_.vertices.size(); ++i )
            {
                trgs_[subdivision_[i]].insert( graph_.vertices[i] );
            }
            if ( use_edges_ )
            {
                util::stopwatch _( "insert constraints" );
                foreach ( typename graph_t::edge_t const & edge, graph_.edges )
                {
                    size_t begin = edge.first, end = edge.second;
                    if ( subdivision_[begin] == subdivision_[end] )
                    {
                        trgs_[subdivision_[begin]].insert_constraint( graph_.vertices[begin], graph_.vertices[end] );
                    }
                }
            }            
        }

        void factorize( size_t subareas_num )
        {
            util::stopwatch _("coordinate vectors factorization");

            analysers_.resize( subareas_num );
            coefficients_.resize( subareas_num );
            for ( size_t s = 0; s != subareas_num; ++s )
            {
                util::stopwatch _( ( std::string("factorize coordinate vectors in subarea ") + lexical_cast< std::string >( s ) ).c_str() );
                coefficients_[s] = coefficients( s );
            }
        }

        void modify_vertices( message_t const & message, size_t chip_rate, int key, double alpha )
        {
            step_ = STEP_SIZE;
            modified_vertices_.clear();

            for ( size_t s = 0; s != subareas_num_; ++s )
            {
                util::stopwatch _( ( std::string("embedding message in subarea ") + lexical_cast< std::string >( s ) ).c_str() );
                vertices_t r = coefficients_[s];

                srand( key );
                assert( chip_rate * message.size() <= r.size() );
                for ( size_t i = 0, k = 0; i != message.size(); ++i )
                {
                    for ( size_t j = 0; j != chip_rate; ++j, ++k )
                    {
                        int p = ( rand() % 2 ) * 2 - 1;
                        int b = message[i] * 2 - 1;
                        r[k] = Point(   r[k].x() + b * p * alpha,
                                        r[k].y() + b * p * alpha );
                    }
                }

                vertices_t new_vertices = analysers_[s]->get_vertices( r );
                std::copy( new_vertices.begin(), new_vertices.end(), std::back_inserter( modified_vertices_ ) ); 
            }
        }

        vertices_t coefficients( size_t subarea )
        {
            trg_t const & trg = trgs_[subarea];
            vertices_t vertices;
            std::map< trg_t::Vertex_handle, size_t > trg_vertex_to_index;
            size_t i = 0;
            typedef trg_t::Finite_vertices_iterator vertices_iterator;
            for ( vertices_iterator v = trg.finite_vertices_begin(); v != trg.finite_vertices_end(); ++v )
            {
                trg_vertex_to_index.insert( std::make_pair( v, i ) );
                vertices.push_back( v->point() );
                ++i;
            }
            analysers_[subarea].reset( new spectral_analyser( incidence_graph( trg, trg_vertex_to_index, weighted_ ) ) );
            return  analysers_[subarea]->get_coefficients( vertices );
        }

        typedef geometry::triangulation_graph< trg_t > incidence_graph;
        typedef boost::shared_ptr< spectral_analyser > analyser_ptr;


        size_t                              subareas_num_;
        graph_t                             graph_;
        std::vector< trg_t >                trgs_;
        std::vector< size_t >               subdivision_;
        vertices_t                          modified_vertices_;
        std::vector< analyser_ptr >         analysers_;
        std::vector< vertices_t >           coefficients_;
        step_t                              step_;

        bool                                weighted_;
        bool                                use_edges_;
    };

    template< class Point >
    std::auto_ptr< embedding_impl< Point > > embed( planar_graph< Point > const & graph, bool weighted, bool use_edges, 
													bool step_by_step = false )
    {
        util::stopwatch _("watermarking generator creation");
        return std::auto_ptr< embedding_impl< Point > >( new embedding_impl< Point >( graph, weighted, use_edges, step_by_step ) );
    }
}

#endif /* _EMBEDDING_H_ */
