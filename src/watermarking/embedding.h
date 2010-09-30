#ifndef _EMBEDDING_H_
#define _EMBEDDING_H_

#include "common.h"

#include <boost/lexical_cast.hpp>

using boost::lexical_cast;

namespace watermarking
{
    template< class Point >
    struct embedding_impl
    {
        typedef planar_graph< Point >                                   graph_t;
        typedef CGAL::Exact_predicates_inexact_constructions_kernel     Gt;
        typedef CGAL::Constrained_Delaunay_triangulation_2< Gt >        trg_t;

        typedef typename graph_t::vertices_t                            vertices_t;

        enum step_t
        {
            SUBDIVIDE_PLANE, BUILD_TRIANGULATIONS, FACTORIZE, MODIFY_VERTICES, STEP_SIZE
        };
        
        embedding_impl( graph_t const & graph, size_t max_patch_size, 
                        bool weighted, bool use_edges, bool step_by_step );

        bool next_step();

        graph_t modified_graph() const
        {
            graph_t res;
            res.vertices = modified_vertices_;
            res.edges = graph_.edges;
            return res;
        }

        std::vector< analyser_ptr > get_analysers() const
        {
            return analysers_;
        }

//    private:

        size_t subdivide_plane( size_t max_subarea_size );

        void build_trgs( size_t subareas_num );

        void factorize( size_t subareas_num );

        void modify_vertices( message_t const & message, size_t chip_rate, int key, double alpha );
 

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


// --------------------------------------------- Implementation -------------------------------------- //

    template< class Point >
    embedding_impl< Point >::embedding_impl(    graph_t const & graph, size_t max_patch_size, 
                                                bool weighted, bool use_edges, bool step_by_step )
            : graph_( graph )
            , max_patch_size_( max_patch_size )
            , weighted_( weighted )
            , use_edges_( use_edges )
    {
        step_ = SUBDIVIDE_PLANE;
        if ( !step_by_step )
            while ( next_step() );
    }

    template< class Point >
    bool embedding_impl< Point >::next_step()
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

    // vector subdivision_ will be filled by indices of zone in which graph_.vertices[i] will be laid.
    template< class Point >
    size_t embedding_impl< Point >::subdivide_plane( size_t max_subarea_size )
    {
        util::stopwatch _( "subidivide plane" );

        // ----------- types -------------
        typedef typename graph_t::vertex_t vertex_t;
        typedef typename graph_t::edge_t edge_t;
        // -------------------------------

        // --------- constants -----------
        const size_t N = vertices_num( graph_ );
        // -------------------------------
        
        std::map< vertex_t, size_t > old_index;
        size_t i = 0;
        foreach ( vertex_t const & v, graph_.vertices )
        {
            old_index.insert( std::make_pair( v, i ) );
            ++i;
        }

        size_t res = geometry::subdivide_plane( graph_.vertices.begin(), graph_.vertices.end(), max_subarea_size, true, subdivision_, 0 );

        std::vector< size_t > old2new( N );
        for ( size_t i = 0; i != N; ++i )
        {
            old2new[old_index[graph_.vertices[i]]] = i;
        }

        foreach ( edge_t & e, graph_.edges )
        {
            e.first = old2new[e.first];
            e.second = old2new[e.second];
        }
        return res;
    }

    template< class Point >
    void embedding_impl< Point >::modify_vertices( message_t const & message, size_t chip_rate, int key, double alpha )
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

    template< class Point >
    void embedding_impl< Point >::build_trgs( size_t subareas_num )
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
            foreach ( typename graph_t::edge_t const & edge,  graph_.edges )
            {
                size_t begin = edge.first, end = edge.second;
                if ( subdivision_[begin] == subdivision_[end] )
                {
                    trgs_[subdivision_[begin]].insert_constraint( graph_.vertices[begin], graph_.vertices[end] );
                }
            }
        }            
    }
    
    template< class Point >
    void embedding_impl< Point >::factorize( size_t subareas_num )
    {
        util::stopwatch _("coordinate vectors factorization");

        // ----------- types -------------
        typedef geometry::triangulation_graph< trg_t >      incidence_graph;
        typedef typename trg_t::Vertex_handle               trg_vertex; 
        typedef typename trg_t::Finite_vertices_iterator    trg_vertices_iterator;
        // -------------------------------

        analysers_.resize( subareas_num );
        coefficients_.resize( subareas_num );
        for ( size_t s = 0; s != subareas_num; ++s )
        {
            std::string step_title = "factorize coordinate vectors in subarea ";
            step_title += lexical_cast< std::string >( s ); 
            util::stopwatch _( step_title.c_str() );

            trg_t const &   trg         = trgs_[s];
            analyser_ptr &  analyser    = analysers_[s];

            vertices_t vertices;        
            std::map< trg_vertex, size_t > trg_vertex_to_index;
            size_t i = 0;

            for ( trg_vertices_iterator v = trg.finite_vertices_begin(); v != trg.finite_vertices_end(); ++v )
            {
                trg_vertex_to_index[v] = i;
                vertices.push_back( v->point() );
                ++i;
            }
            
            incidence_graph graph( trg, trg_vertex_to_index, weighted_ );
            boost::function< void (incidence_graph const &, spectral_analyser::matrix_t &) > fill_matrix =
                boost::bind( &watermarking::details::fill_matrix_by_chen< incidence_graph, spectral_analyser::matrix_t >, _1, _2 ); 
            analyser.reset( new spectral_analyser( graph, fill_matrix ) );
        
            coefficients_[s] = analyser->get_coefficients( vertices );
        }
    }

    template< class Point >
    std::auto_ptr< embedding_impl< Point > > embed( planar_graph< Point > const & graph, size_t max_patch_size, bool weighted, bool use_edges, 
													bool step_by_step = false )
    {
        util::stopwatch _("watermarking generator creation");

        typedef embedding_impl< Point > result_t; 
        return std::auto_ptr< result_t >( new result_t( graph, max_patch_size, weighted, use_edges, step_by_step ) );
    }
}

#endif /* _EMBEDDING_H_ */
