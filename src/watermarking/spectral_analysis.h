#ifndef _SPECTRAL_ANALYSIS_H_
#define _SPECTRAL_ANALYSIS_H_

#include <tnt/tnt_array2d.h>
#include <jama/jama_eig.h>

namespace watermarking
{
    namespace
    {
        typedef TNT::Array2D< double > matrix_t;

        template< class Graph >
        matrix_t adjacency_matrix( Graph const & graph )
        {
            const size_t N = graph.vertices_num();
            matrix_t a( N, N );
            for ( size_t v = 0; v != N; ++v )
            {
                std::fill_n( &a[v][0], N, 0 );
                a[v][v] = 1;
                size_t d = graph.degree( v );
                for ( typename Graph::edges_iterator e = graph.edges_begin( v ); e != graph.edges_end( v ); ++e )
                    a[v][*e] = -1.0 / d;
            }
            return a;
        }

        template< class Points >
        Points spectral_coefficients( Points const & vertices, matrix_t const & eigen_vectors )
        {
            const size_t N = vertices.size();
            Points coefficients( N );
            for ( size_t i = 0; i != N; ++i )
            {
                typedef typename Points::value_type point_t;
                double x = 0, y = 0;
                for ( size_t j = 0; j != N; ++j )
                {
                    x += eigen_vectors[i][j] * vertices[j].x();
                    y += eigen_vectors[i][j] * vertices[j].y();
                }
                coefficients[i] = point_t( x, y );
            }
            return coefficients;
        }
    }

    template< class Graph, class Points >
    Points spectral_coefficients( Graph const & graph, Points const & vertices )
    {
        matrix_t a = adjacency_matrix( graph );
        const size_t N = graph.vertices_num();
        matrix_t w( N, N );
        JAMA::Eigenvalue< double >( a ).getV( w );
        return spectral_coefficients( vertices, w );
    }
}

#endif /* _SPECTRAL_ANALYSIS_H_ */
