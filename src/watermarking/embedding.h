#ifndef _EMBEDDING_H_
#define _EMBEDDING_H_

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_2.h>
#include <CGAL/Constrained_Delaunay_triangulation_2.h>

#include "spectral_analysis.h"
#include "../geometry/plane_subdivision.h"
#include "../geometry/triangulation_graph.h"

namespace watermarking
{
    const size_t MAX_PATCH_SIZE = 100;

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
        typedef CGAL::Exact_predicates_inexact_constructions_kernel     Kernel;
        typedef CGAL::Delaunay_triangulation_2< Kernel >                DT;
        typedef CGAL::Constrained_Delaunay_triangulation_2< Kernel >    CDT;
        typedef DT                                                      trg_t;
        typedef typename graph_t::vertices_t                            vertices_t;

        enum step_t
        {
            SUBDIVIDE_PLANE, BUILD_TRIANGULATIONS, MODIFY_VERTICES, STEP_SIZE
        };
        
        embedding_impl( graph_t const & graph, message_t const & message, int chip_rate, int key, double alpha, bool step_by_step = false )
                : graph_( graph )
                , message_( message )
                , chip_rate_( chip_rate )
                , key_( key )
                , alpha_( alpha )
        {
            step_ = SUBDIVIDE_PLANE;
            while ( step_by_step && next_step() );
        }

        bool next_step()
        {
            switch ( step_ )
            {
            case SUBDIVIDE_PLANE:
                subareas_num_ = subdivide_plane( MAX_PATCH_SIZE );
                step_ = BUILD_TRIANGULATIONS;
                return true;
            case BUILD_TRIANGULATIONS:
                build_trgs( subareas_num_ );
                step_ = MODIFY_VERTICES;
                return true;
            case MODIFY_VERTICES:
                modify_vertices( subareas_num_, message_, chip_rate_, key_, alpha_ );
                step_ = STEP_SIZE;
                return true;
            }
            return false;
        }

//    private:

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

        void modify_vertices( size_t subareas_num, message_t const & message, int chip_rate, int key, double alpha )
        {
            for ( size_t s = 0; s != subareas_num; ++s )
            {
                std::cout << "embedding message in subarea " << s << std::endl;
                vertices_t r = coefficients( s );

                srand( key );
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

                modified_vertices_[s] = analysers_[s]->get_vertices( r );
            }
        }

        void build_trgs( size_t subareas_num )
        {
            std::cout << "build_triangulations\n";
            trgs_.resize                ( subareas_num );
            analysers_.resize           ( subareas_num );
            modified_vertices_.resize   ( subareas_num );

            for ( size_t i = 0; i != graph_.vertices.size(); ++i )
            {
                trgs_[subdivision_[i]].insert( graph_.vertices[i] );
            }
            /*
            foreach ( typename graph_t::edge_t const & edge, graph_.edges )
            {
                size_t begin = index_[edge.first], end = index_[edge.second];
                if ( subdivision_[begin] == index_[end] )
                {
                    trgs_[subdivision_[begin]].insert_constraint( graph_.vertices[begin], graph_.vertices[end] );
                }
            }
            */
        }

        vertices_t coefficients( size_t subarea )
        {
            trg_t const & trg = trgs_[subarea];
            vertices_t vertices( trg.number_of_vertices() );
            std::map< trg_t::Vertex_handle, size_t > trg_vertex_to_index;
            size_t i = 0;
            typedef trg_t::Finite_vertices_iterator vertices_iterator;
            for ( vertices_iterator v = trg.vertices_begin(); v != trg.vertices_end(); ++v )
            {
                trg_vertex_to_index.insert( std::make_pair( v, i ) );
                vertices.push_back( v->point() );
                ++i;
            }
            analysers_[i].reset( new spectral_analyser< incidence_graph >( incidence_graph( trg, trg_vertex_to_index ) ) );
            return  analysers_[i]->get_coefficients( vertices );
        }

        typedef geometry::triangulation_graph< DT >                         incidence_graph;
        typedef boost::shared_ptr< spectral_analyser< incidence_graph > >   analyser_ptr;

        size_t                              subareas_num_;
        std::vector< size_t >               index_;
        graph_t                             graph_;
        std::vector< trg_t >                trgs_;
        std::vector< size_t >               subdivision_;
        std::vector< vertices_t >           modified_vertices_;
        std::vector< analyser_ptr >         analysers_;
        step_t                              step_;

        message_t                           message_;
        int                                 chip_rate_;
        int                                 key_;
        double                              alpha_;
    };

    template< class Point >
    std::auto_ptr< embedding_impl< Point > > embed( planar_graph< Point > const & graph, message_t const & message )
    {
        return std::auto_ptr< embedding_impl< Point > >( new embedding_impl< Point >( graph, message, 3, 239, 1.5 ) );
    }
}

#endif /* _EMBEDDING_H_ */
