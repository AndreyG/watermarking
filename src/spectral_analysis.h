#ifndef _SPECTRAL_ANALYSIS_H_
#define _SPECTRAL_ANALYSIS_H_

#include "utility/stopwatch.h"
#include "geometry/point.h"

namespace watermarking
{
    struct spectral_analyser
    {
        typedef geometry::point_t           vertex_t;
        typedef std::vector< vertex_t >     vertices_t;
        typedef std::vector< double >       coefficients_t;
        
        virtual ~spectral_analyser() {};
        virtual vertices_t      get_vertices    ( coefficients_t    const & ) const = 0;
        virtual coefficients_t  get_coefficients( vertices_t        const & ) const = 0;
		virtual void			dump			( std::ostream &			) const = 0;
    };

    template< class Traits >
    struct spectral_analyser_impl : spectral_analyser
    {
        typedef typename Traits::vector_t vector_t;

	protected:

		template< class Graph >
        spectral_analyser_impl( Graph const & g )
                : N ( g.vertices_num() )
                , e_( N * N )
        {}

	public:

        explicit spectral_analyser_impl( std::istream & in )
                : N( read_size( in ) )
				, e_(N * N)
        {
            util::stopwatch _("spectral analyser: reading from file");

            for (size_t i = 0; i != N; ++i)
            {
                for (size_t j = 0; j != N; ++j)
                    in >> e_[i * N + j];
            }
        }

        void dump( std::ostream & out ) const
        {
            for (size_t i = 0; i != N; ++i) 
            {
                for (size_t j = 0; j != N; ++j) 
                {
                    out << e_[i * N + j] << " ";
                }
                out << std::endl;
            }
        }

    private:

        size_t read_size( std::istream & in )
        {
            size_t s;
            in >> s;
            return s;
        }

    public:

        vertices_t get_vertices( coefficients_t const & r ) const
        {
            const size_t K = r.size();
            assert( K <= N );
            
            vertices_t vertices( N );
            for ( size_t i = 0; i != N; ++i )
            {
                for ( size_t j = 0; j != K; ++j )
                {
                    vertices[i] += Traits::mult( r[j], e_[i * N + j] );
                }
            }

            return vertices;
        }

        coefficients_t get_coefficients( vertices_t const & vertices ) const 
        {
            assert( vertices.size() == N );

            auto pts = Traits::points_to_flat( vertices );   
            coefficients_t coefficients(N);
            for ( size_t i = 0; i != N; ++i )
            {
                coefficients[i] = Traits::get_coeff( &e_[0] + i * N, N, pts );
            }
            return coefficients;
        }

	protected:

		size_t N;
        vector_t e_;
    };

    struct real_traits
    {
		typedef std::vector< double > vector_t;

        static geometry::point_t mult( double r, double e )
        {
            return geometry::point_t( r * e, r * e );
        }

        typedef boost::tuple< vector_t, vector_t > points_to_flat_type;

        static points_to_flat_type points_to_flat( std::vector< geometry::point_t > const & pts )
        {
            const size_t N = pts.size();
            vector_t x(N), y(N);
            for ( size_t i = 0; i != N; ++i )
            {
                x[i] = pts[i].x();
                y[i] = pts[i].y();
            }

            return points_to_flat_type(x, y);
        }

        static double get_coeff( double const * e, size_t N, points_to_flat_type const & pts )
        {
			vector_t const & a = boost::get< 0 >( pts );
			vector_t const & b = boost::get< 1 >( pts );
			assert(a.size() == N);
			assert(b.size() == N);
			return dot_prod(&a[0], e, N) + dot_prod(&b[0], e, N);
        }

	private:

		static double dot_prod( double const * a, double const * b, const size_t N )
		{
			double res = 0;
			for ( size_t i = 0; i != N; ++i, ++a, ++b )
			{
				res += (*a * *b);
			}
			return res;
		}
    };

/*
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
                const double d = graph.degree( v );
                for ( typename Graph::edges_iterator e = graph.edges_begin( v ); e != graph.edges_end( v ); ++e )
                    a(v, e->end) = -e->weight / d;
            }
        }
    }
  	*/       
}

#endif /* _SPECTRAL_ANALYSIS_H_ */
