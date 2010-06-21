#ifndef _EMBEDDING_H_
#define _EMBEDDING_H_

#include "common.h"

#include <boost/lexical_cast.hpp>

using boost::lexical_cast;

namespace watermarking
{
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
        
        embedding_impl( graph_t const & graph, size_t max_patch_size, bool weighted, bool use_edges, bool step_by_step )
                : graph_( graph )
                , max_patch_size_( max_patch_size )
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
                subareas_num_ = subdivide_plane( max_patch_size_ );
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
            return watermarking::subdivide_plane( graph_, max_subarea_size, subdivision_ );
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
            util::stopwatch _("embedding message");

            step_ = STEP_SIZE;
            modified_vertices_.clear();

            for ( size_t s = 0; s != subareas_num_; ++s )
            {
                vertices_t r = coefficients_[s];

                srand( key );
                if ( chip_rate * message.size() > r.size() )
                {
                    std::cout << chip_rate * message.size() << "\t" << r.size() << std::endl;
                    assert( false );
                }
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

        graph_t modified_graph() const
        {
            graph_t res;
            res.vertices = modified_vertices_;
            res.edges = graph_.edges;
            return res;
        }

        vertices_t coefficients( size_t subarea )
        {
            typedef typename vertices_t::value_type vertex_t; 
            return watermarking::coefficients< trg_t, vertex_t >( trgs_[subarea], analysers_[subarea], weighted_ );
        }

        size_t                              subareas_num_;
        graph_t                             graph_;
        std::vector< trg_t >                trgs_;
        std::vector< size_t >               subdivision_;
        vertices_t                          modified_vertices_;
        std::vector< analyser_ptr >         analysers_;
        std::vector< vertices_t >           coefficients_;
        step_t                              step_;

        size_t                              max_patch_size_;
        bool                                weighted_;
        bool                                use_edges_;
    };

    template< class Point >
    std::auto_ptr< embedding_impl< Point > > embed( planar_graph< Point > const & graph, size_t max_patch_size, bool weighted, bool use_edges, 
													bool step_by_step = false )
    {
        util::stopwatch _("watermarking generator creation");
        return std::auto_ptr< embedding_impl< Point > >( new embedding_impl< Point >( graph, max_patch_size, weighted, use_edges, step_by_step ) );
    }
}

#endif /* _EMBEDDING_H_ */
