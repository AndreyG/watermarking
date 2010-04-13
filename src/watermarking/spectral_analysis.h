#ifndef _SPECTRAL_ANALYSIS_H_
#define _SPECTRAL_ANALYSIS_H_

#include <tnt/tnt_array2d.h>
#include <jama/jama_eig.h>

namespace watermarking
{
    namespace
    {
        typedef TNT::Array2D< double > matrix_t;

        template< class Iter >
            typename std::iterator_traits< Iter >::value_type 
                norm( Iter begin, Iter end )
        {
            typename std::iterator_traits< Iter >::value_type res = 0;
            for ( ; begin != end; ++end )
                res += ( *begin ) * ( *begin );
            return res;
        }

        template< class Iter >
        void normalize( Iter begin, Iter end )
        {
            typename std::iterator_traits< Iter >::value_type n = norm( begin, end );
            for ( ; begin != end; ++begin )
                *begin /= n;
        }

        void normalize( matrix_t & a )
        {
            for ( size_t i = 0; i != a.dim1(); ++i )
            {
                normalize( &a[i][0], &a[i][0] + a.dim2() );
            }
        }

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

    template< class Graph >
    struct spectral_analyser
    {
        spectral_analyser( Graph const & graph )
                : e_( graph.vertices_num(), graph.vertices_num() )
        {
            matrix_t a = adjacency_matrix( graph );             
            JAMA::Eigenvalue< double >( a ).getV( e_ );
            normalize( e_ );
        }

        template< class Points >
        Points get_coefficients( Points const & vertices ) const
        {
            return spectral_coefficients( vertices, e_ );
        }

        template< class Points >
        Points get_vertices( Points const & coefficients ) const
        {
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

}

#endif /* _SPECTRAL_ANALYSIS_H_ */
