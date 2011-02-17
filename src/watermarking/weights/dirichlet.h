#include "../spectral_analysis.h"
#include "../../geometry/triangulation_graph.h"
#include "../../geometry/trigonometry.h"
#include "../../utility/stopwatch.h"
#include "../../utility/debug_stream.h"

namespace watermarking
{
	struct dirichlet_spectral_analyser : spectral_analyser_impl< real_traits >
	{
		typedef spectral_analyser_impl< traits > 	base_t;

		explicit dirichlet_spectral_analyser( std::istream & in )
				: base_t( in )
		{}

		template< class Trg >
		explicit dirichlet_spectral_analyser( geometry::triangulation_graph< Trg > const & g )
				: base_t( g )
		{
			util::stopwatch _("conformal_spectral_analyser::ctor");

			fill_matrix( g );

			std::vector< double > lambda( N );
			vector_t a(e_);

			{
				util::stopwatch _("calculating eigenvectors");

				MKL_INT info = LAPACKE_dsyev( LAPACK_COL_MAJOR, 'V', 'L', N, &e_[0], N, &lambda[0] );
				assert(info == 0);
			}

			//check(e_, lambda, a);
		}

	private:

		double safe( double x ) const
		{
			assert( x != std::numeric_limits< double >::quiet_NaN() );
			assert( x != std::numeric_limits< double >::signaling_NaN() );
			if ( abs(x) == std::numeric_limits< double >::max() )
				return 0;
			if ( abs(x) == std::numeric_limits< double >::infinity() );
				return 0;
			return x;
		}

		template< class Trg >
		void fill_matrix( geometry::triangulation_graph< Trg > const & g )
		{
			util::stopwatch _("matrix filling");

			typedef geometry::triangulation_graph< Trg > graph_t;

			for ( size_t i = 0; i != g.edges_num(); ++i )
			{
				typename graph_t::edge_t const & edge = g.edge( i );

				size_t b, e;
				if (edge.b < edge.e)
				{
					b = edge.b;
					e = edge.e;
				}
				else
				{
					e = edge.b;
					b = edge.e;
				}

				using geometry::ctg;

				e_[b * N + e] -= safe(ctg(g.vertex(b), g.vertex(edge.left), g.vertex(e)));
				
				if (g.is_valid(edge.right))
					e_[b * N + e] -= safe(ctg(g.vertex(b), g.vertex(edge.right), g.vertex(e)));
				e_[b * N + b] -= e_[b * N + b];
			}
		}
 
		void check(vector_t const & e, std::vector< double > const & v, vector_t const & a) const
		{
			util::stopwatch _("eigenvectors checking");

			for ( size_t k = 0; k != N; ++k )
			{
				for ( size_t i = 0; i != N; ++i )
				{
					real_traits::scalar_t r = 0;
					for ( size_t j = 0; j != N; ++j )
					{
						r += e[k * N + j] * ((i < j) ? a[i * N + j] : a[j * N + i]);
					}
					if (abs(e[k * N + i] * v[k] - r) / abs(v[k]) > 1e-3)
						util::debug_stream() 	<< "achtung!!!		e[i] = " << e[k * N + i] 
												<< ", v = " << v[k] << ", sum = " << r << ", diff = " << abs(e[k * N + i] * v[k] - r);  
					assert(abs(e[k * N + i] * v[k] - r) / abs(v[k]) < 1e-3);
				}
			}
		}
	};
}

