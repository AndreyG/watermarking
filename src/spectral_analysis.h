#ifndef _SPECTRAL_ANALYSIS_H_
#define _SPECTRAL_ANALYSIS_H_

#include <lapackpp.h>

#include "utility/stopwatch.h"

namespace watermarking
{
    
    namespace
    {
        template< class Points, class Matrix >
        Points spectral_coefficients( Points const & vertices, Matrix const & eigen_vectors )
        {
            const size_t N = vertices.size();
            assert( eigen_vectors.cols() == (int) N );
            assert( eigen_vectors.rows() == (int) N );

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
                assert( abs( Blas_Norm2( e ) - 1.0 ) < 1e-5 );
                coefficients[i] = point_t(  Blas_Dot_Prod( x, e ),
                                            Blas_Dot_Prod( y, e ) );
            }
            return coefficients;
        }
    }

    namespace details
    {
        template< class Matrix >
        void check_symmetry( Matrix const & a )
        {
            const size_t N = a.rows();
            for ( size_t v = 0; v + 1 != N; ++v )
            {
                for ( size_t u = v + 1; u != N; ++u )
                    assert( a(v, u) == a(u, v) );
            }
        }

        template< class Graph, class Matrix >
        void fill_matrix_by_chen( Graph const & graph, Matrix & a )
        {
            util::stopwatch _("creation of adjacency matrix");
            const size_t N = graph.vertices_num();

            for ( size_t v = 0; v != N; ++v )
            {
                for ( size_t u = 0; u != N; ++u )
                    a(v, u) = 0;
                a(v, v) = graph.degree( v );
                for ( typename Graph::edges_iterator e = graph.edges_begin( v ); e != graph.edges_end( v ); ++e )
                    a(v, e->end) = -e->weight;
            }
            check_symmetry( a );
        }

        template< class Graph, class Matrix >
        void fill_matrix_by_obuchi( Graph const & graph, Matrix & a )
        {
            util::stopwatch _("creation of adjacency matrix");
            const size_t N = graph.vertices_num();

            for ( size_t v = 0; v != N; ++v )
            {
                for ( size_t u = 0; u != N; ++u )
                    a(v, u) = 0;
                a(v, v) = 1;
                double d = graph.degree( v );
                for ( typename Graph::edges_iterator e = graph.edges_begin( v ); e != graph.edges_end( v ); ++e )
                    a(v, e->end) = -e->weight / d;
            }
        }
    }
         
    struct spectral_analyser
    {
        typedef LaGenMatDouble matrix_t;

        template< class Graph >
        spectral_analyser(  Graph const & graph, boost::function< void (Graph const &, matrix_t &) > fill_matrix )
                : lambda_( graph.vertices_num() )
                , e_( graph.vertices_num(), graph.vertices_num() )
        {
            util::stopwatch _("calculating eigenvectors");
            const size_t N = graph.vertices_num();
            fill_matrix( graph, e_ );

            LaEigSolveSymmetricVecIP( e_, lambda_ );
            for ( size_t i = 0; i + 1 != N; ++i )
            {
                assert( lambda_(i) <= lambda_(i + 1) );
            }
        }

        template< class Stream >
        void dump( Stream & out ) const
        {
            const int N = lambda_.size();
            out << N << std::endl;
            for (int i = 0; i != N; ++i) 
            {
                for (int j = 0; j != N; ++j) 
                {
                    out << e_(i, j) << " ";
                }
                out << std::endl;
            }
            for (int i = 0; i != N; ++i)
                out << lambda_(i) << " ";
        }

    private:
        template< class Stream >
        int read_size( Stream & in )
        {
            int s;
            in >> s;
            return s;
        }
    public:

        template< class Stream >
        explicit spectral_analyser( Stream & in )
                : lambda_( read_size(in) )
                , e_(lambda_.size(), lambda_.size())
        {
            util::stopwatch _("spectral analyser: reading from file");

            const int N = lambda_.size();
            for (int i = 0; i != N; ++i)
            {
                for (int j = 0; j != N; ++j)
                    in >> e_(i, j);
            }
            for (int i = 0; i != N; ++i)
                in >> lambda_(i);
        }


        double get_lambda( size_t i ) const
        {
            return lambda_( i );
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
            assert( (int) N == e_.rows() );
            assert( (int) N == e_.cols() );
            
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
        LaVectorDouble lambda_;
        matrix_t e_;
    };
}

#endif /* _SPECTRAL_ANALYSIS_H_ */