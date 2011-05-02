#ifndef _SPECTRAL_ANALYSIS_H_
#define _SPECTRAL_ANALYSIS_H_

#include "../utility/stopwatch.h"
#include "../geometry/point.h"

namespace watermarking
{
    struct spectral_analyser
    {
        typedef geometry::point_t           vertex_t;
        typedef std::vector< vertex_t >     vertices_t;
        typedef std::vector< double >       coefficients_t;
        
        virtual ~spectral_analyser() {};

        virtual size_t          vectors_num ()                                  const = 0;
        virtual vertices_t      get_vertices    ( coefficients_t    const & )   const = 0;
        virtual coefficients_t  get_coefficients( vertices_t        const & )   const = 0;

		virtual void			dump			( std::ostream &			)   const = 0;
    };

    template< class Traits >
    struct spectral_analyser_impl : spectral_analyser
    {
	protected:
        typedef typename Traits::vector_t vector_t;
		typedef Traits traits;

		template< class Graph >
        spectral_analyser_impl(Graph const & g)
                : N ( g.vertices_num() )
                , e_( N * N )
        {}

	public:

        explicit spectral_analyser_impl( std::istream & in )
                : N( read_number( in ) )
                , K( read_number( in ) )
				, e_(N * N)
        {
            util::stopwatch _("spectral analyser: reading from file");

            for (size_t i = 0; i != K; ++i)
            {
                for (size_t j = 0; j != N; ++j)
                    in >> e_[i * N + j];
            }
        }

        void dump( std::ostream & out ) const
        {
			out << N << " " << K << std::endl;
            for (size_t i = 0; i != K; ++i) 
            {
                for (size_t j = 0; j != N; ++j) 
                {
                    out << e_[i * N + j] << " ";
                }
                out << std::endl;
            }
        }

    private:

        size_t read_number( std::istream & in )
        {
            size_t s;
            in >> s;
            return s;
        }

    public:
        // vectors_num -- число собственных векторов, которое мы смогли вычислить
        size_t vectors_num() const { return K; }

        vertices_t get_vertices( coefficients_t const & r ) const
        {
            assert(r.size() <= K);
            
            vertices_t vertices( N );
            for (size_t i = 0; i != N; ++i)
            {
                for (size_t k = 0; k != r.size(); ++k)
                {
                    vertices[i] += Traits::mult(r[k], e_[k * N + i]);
                }
            }

            return vertices;
        }

        coefficients_t get_coefficients( vertices_t const & vertices ) const 
        {
            assert(vertices.size() == N);

            auto pts = Traits::points_to_flat(vertices);   
            coefficients_t coefficients(N);
            for ( size_t i = 0; i != N; ++i )
            {
                coefficients[i] = Traits::get_coeff(&e_[0] + i * N, N, pts);
            }
            return coefficients;
        }

	protected:

		size_t N;
        size_t K;
        vector_t e_;
    };

	namespace
	{
		template< class Scalar >
		Scalar dot_prod( Scalar const * a, Scalar const * b, const size_t N )
		{
			Scalar res = 0;
			for ( size_t i = 0; i != N; ++i, ++a, ++b )
			{
				res += (*a * *b);
			}
			return res;
		}
	}

    struct real_traits
    {
		typedef double 						scalar_t;
		typedef std::vector< scalar_t > 	vector_t;

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

        static double get_coeff( scalar_t const * e, const size_t N, points_to_flat_type const & pts )
        {
			vector_t const & a = boost::get< 0 >( pts );
			vector_t const & b = boost::get< 1 >( pts );
			assert(a.size() == N);
			assert(b.size() == N);
			return 	dot_prod< scalar_t >(&a[0], e, N) + 
					dot_prod< scalar_t >(&b[0], e, N);
        }
    };

	struct complex_traits
	{
		typedef std::complex< double > 		scalar_t;
		typedef std::vector< scalar_t > 	vector_t;

		typedef vector_t points_to_flat_type;

		static points_to_flat_type points_to_flat( std::vector< geometry::point_t > const & pts )
		{
			points_to_flat_type	res;
			foreach ( geometry::point_t const & pt, pts )
				res.push_back( scalar_t( pt.x(), -pt.y() ) );
			return res;
		}

		static geometry::point_t mult( double r, scalar_t e )
		{
			return geometry::point_t( r * e.real(), r * e.imag() );
		}

		static double get_coeff( scalar_t const * e, const size_t N, points_to_flat_type const & pts )
        {
			scalar_t r = dot_prod< scalar_t >(e, &pts[0], N);
			return r.real();
		}
	};
}

#endif /* _SPECTRAL_ANALYSIS_H_ */
