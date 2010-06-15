#ifndef _SPECTRAL_ANALYSIS_H_
#define _SPECTRAL_ANALYSIS_H_

#include <lapackpp.h>

namespace watermarking
{
    
    namespace
    {
        template< class Graph, class Matrix >
        void fill_adjacency_matrix( Graph const & graph, Matrix & a )
        {
            util::stopwatch _("creation of adjacency matrix");
            const size_t N = graph.vertices_num();

            for ( size_t v = 0; v != N; ++v )
            {
                for ( size_t u = 0; u != N; ++u )
                    a(v, u) = 0;
                a(v, v) = 1;
                size_t d = graph.degree( v );
                for ( typename Graph::edges_iterator e = graph.edges_begin( v ); e != graph.edges_end( v ); ++e )
                    a(v, e->end) = -e->weight / d;
            }
        }

        template< class Points, class Matrix >
        Points spectral_coefficients( Points const & vertices, Matrix const & eigen_vectors )
        {
            const size_t N = vertices.size();
            assert( eigen_vectors.cols() == N );
            assert( eigen_vectors.rows() == N );

            LaVectorDouble x(N), y(N);
            for ( size_t i = 0; i != N; ++i )
            {
                x(i) = vertices[i].x();
                y(i) = vertices[i].y();
            }
            Points coefficients(N);
            for ( size_t i = 0; i != N; ++i )
            {
                typedef typename Points::value_type point_t;
                LaVectorDouble e(N);
                for ( size_t j = 0; j != N; ++j)
                    e(j) = eigen_vectors(j, i);
                coefficients[i] = point_t(  Blas_Dot_Prod( x, e ),
                                            Blas_Dot_Prod( y, e ) );
            }
            return coefficients;
        }
    }

    struct spectral_analyser
    {
        template< class Graph >
        spectral_analyser( Graph const & graph )
                : e_( graph.vertices_num(), graph.vertices_num() )
        {
            util::stopwatch _("calculating eigenvectors");
            const size_t N = graph.vertices_num();
            LaSpdMatDouble A(N, N);
            fill_adjacency_matrix( graph, A );

            LaVectorDouble lambda_real(N), lambda_imag(N);
            LaEigSolve( A, lambda_real, lambda_imag, e_ );
        }

        template< class Points >
        Points get_coefficients( Points const & vertices ) const
        {
            return spectral_coefficients( vertices, e_ );
        }

        template< class Points >
        Points get_vertices( Points const & coefficients ) const
        {
            typedef typename Points::value_type point_t;
            
            const size_t N = coefficients.size();
            assert( N == e_.rows() );
            assert( N == e_.cols() );
            
            Points vertices( N );
            for ( size_t i = 0; i != N; ++i )
            {
                double x = 0, y = 0;
                for ( size_t j = 0; j != N; ++j )
                {
                    point_t const & r = coefficients[j];
                    x += r.x() * e_(i, j);
                    y += r.y() * e_(i, j);
                }
                vertices[i] = point_t( x, y );
            }

            return vertices;
        }

    private:
        typedef LaGenMatDouble matrix_t;
        matrix_t e_;
    };
    
    /*
    namespace 
    {
        double sqr( double x )
        {
            return x * x;
        }

        double norm( std::vector< double > const & a )
        {
            double res = 0.0;
            foreach( double x, a )
                res += sqr( x );
            return sqrt( res );
        }

        void normalize( std::vector< double > & a )
        {
            const double n = norm( a );
            foreach( double & x, a )
                x /= n;
        }

        double scalar_product( std::vector< double > const & a, std::vector< double > const & b )
        {
            const int N = a.size();
            assert( N == b.size() );
            double res = 0.0;
            for ( size_t i = 0; i != N; ++i )
                res += a[i] * b[i];
            return res;
        }

        const double EPS = 1e-3;
    }

    struct spectral_analyser
    {
        template< class Graph >
        spectral_analyser( Graph const & graph )
            : e_( graph.vertices_num() )
        {
            const int N = graph.vertices_num();
            e_[0].assign( N, 1.0 / sqrt( N ) );
            for ( size_t i = 1; i != N; ++i )
            {
                e_[i].assign( N, 0 );
                std::vector< double > e( N );
                foreach ( double & x, e )
                    x = ((double ) rand()) / RAND_MAX;
                normalize( e );
                size_t iter = 0;
                do
                {
                    e_[i].swap( e );
                    for ( size_t j = 0; j != i; ++j )
                    {
                        std::vector< double > tmp( e_[j] );
                        for ( size_t k = 0; k != N; ++k )
                            tmp[k] /= graph.degree( k );
                        const double c =    scalar_product( e_[i], tmp ) / 
                                            scalar_product( e_[j], tmp );
                        for ( size_t k = 0; k != N; ++k )
                            e_[i][k] -= c * e_[j][k];
                    }
                    for ( size_t v = 0; v != N; ++v )
                    {
                        double w = 0;
                        for ( typename Graph::edges_iterator it = graph.edges_begin( v ); it != graph.edges_end( v ); ++it )
                            w += it->weight * e_[i][it->end];
                        w /= graph.degree( v );
                        e[v] = 0.5 * ( e_[i][v] + w );
                    }
                    normalize( e );
                } while ( scalar_product( e, e_[i] ) < 1.0 - EPS && ++iter != 100 );
            }
        }

        template< class Points >
        Points get_coefficients( Points const & vertices )
        {
            const size_t N = vertices.size();
            assert( e_.size() == N );
            
            std::vector< double > x( N ), y( N );
            for ( size_t j = 0; j != N; ++j )
            {
                x[j] = vertices[j].x();
                y[j] = vertices[j].y();
            }
            
            Points coefficients( N );
            for ( size_t i = 0; i != N; ++i )
            {
                typedef typename Points::value_type point_t;
                coefficients[i] = point_t(  scalar_product( x, e_[i] ),
                                            scalar_product( y, e_[i] ) );
            }
            return coefficients;
        }

        template< class Points >
        Points get_vertices( Points const & coefficients ) const
        {
            typedef typename Points::value_type point_t;
            
            const size_t N = coefficients.size();
            assert( N == e_.size() );
            
            Points vertices( N );
            for ( size_t i = 0; i != N; ++i )
            {
                double x = 0, y = 0;
                for ( size_t j = 0; j != N; ++j )
                {
                    point_t const & r = coefficients[j];
                    x += r.x() * e_[j][i];
                    y += r.y() * e_[j][i];
                }
                vertices[i] = point_t( x, y );
            }

            return vertices;
        }

    private:
        std::vector< std::vector< double > > e_;
    };
    */

}

#endif /* _SPECTRAL_ANALYSIS_H_ */
