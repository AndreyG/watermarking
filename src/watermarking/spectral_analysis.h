#ifndef _SPECTRAL_ANALYSIS_H_
#define _SPECTRAL_ANALYSIS_H_

#include <tnt/tnt_array2d.h>
#include <jama/jama_eig.h>

namespace watermarking
{
    namespace
    {
        typedef TNT::Array2D< double > matrix_t;

        template< class Scalar >
        Scalar sqr( Scalar x )
        {
            return x * x;
        }

        void normalize( matrix_t & a )
        {
            for ( size_t i = 0; i != a.dim1(); ++i )
            {
                double length = 0;
                for ( size_t j = 0; j != a.dim2(); ++j )
                    length += sqr( a[i][j] );
                length = sqrt( length );
                if ( length == 0 )
                    continue;
                for ( size_t j = 0; j != a.dim2(); ++j )
                    a[i][j] /= length;
            }
        }

        template< class Graph >
        matrix_t adjacency_matrix( Graph const & graph )
        {
            std::cout << "adjacency matrix creation" << std::endl;
            const size_t N = graph.vertices_num();
            matrix_t a( N, N );
            for ( size_t v = 0; v != N; ++v )
            {
                for ( size_t u = 0; u != N; ++u )
                    a[v][u] = 0;
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
            std::cout << "spectral coefficients" << std::endl;
            const size_t N = vertices.size();
            assert( eigen_vectors.dim1() == N );
            assert( eigen_vectors.dim2() == N );
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

    template< class Graph >
    struct spectral_analyser
    {
        spectral_analyser( Graph const & graph )
                : e_( graph.vertices_num(), graph.vertices_num() )
        {
            matrix_t a = adjacency_matrix( graph );             
            std::cout << "eigenvalues calculating" << std::endl;
            JAMA::Eigenvalue< double >( a ).getV( e_ );
            std::cout << "eigenvectors normalizing" << std::endl;
            normalize( e_ );
            std::cout << "spectral_analyser::ctor finished" << std::endl;
        }

        template< class Points >
        Points get_coefficients( Points const & vertices ) const
        {
            return spectral_coefficients( vertices, e_ );
        }

        template< class Points >
        Points get_vertices( Points const & coefficients ) const
        {
            std::cout << "spectral_analyser::get_vertices" << std::endl;
            const size_t N = coefficients.size();
            assert( N == e_.dim1() );
            assert( N == e_.dim2() );
            Points vertices( N );
            for ( size_t i = 0; i != N; ++i )
            {
                typedef typename Points::value_type point_t;
                point_t const & r = coefficients[i];
                double const * e = e_[i];
                double x = 0, y = 0;
                for ( size_t j = 0; j != N; ++j )
                {
                    x += r.x() * e[j];
                    y += r.y() * e[j];
                }
                vertices[i] = point_t( x, y );
            }
        }

    private:
        matrix_t e_;
    };

    template< class Graph >
    spectral_analyser< Graph > * get_spectral_analyser( Graph const & g )
    {
        return new spectral_analyser< Graph >( g );
    }
}

#endif /* _SPECTRAL_ANALYSIS_H_ */
