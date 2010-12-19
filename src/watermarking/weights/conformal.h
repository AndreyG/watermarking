#include "../spectral_analysis.h"
#include "../../geometry/triangulation_graph.h"
#include "../../geometry/trigonometry.h"

namespace watermarking
{
	struct conformal_spectral_analyser : spectral_analyser_impl< complex_traits >
	{
		typedef spectral_analyser_impl< complex_traits > 	base_t;

		explicit conformal_spectral_analyser( std::istream & in )
				: base_t( in )
		{}

		template< class Trg >
		explicit conformal_spectral_analyser( geometry::triangulation_graph< Trg > const & g )
				: base_t( g )
		{
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

				e_[b * N + e] -= ctg(g.vertex(b), g.vertex(edge.left), g.vertex(e));
				
				if (g.is_valid(edge.right))
					e_[b * N + e] -= ctg(g.vertex(b), g.vertex(edge.right), g.vertex(e));
				e_[b * N + b] -= e_[b * N + b];
				
				if (!g.is_valid(edge.right))
				{
					if (edge.b < edge.e)
						e_[edge.b * N + edge.e] -= complex_traits::scalar_t(0, 1);
					else
						e_[edge.e * N + edge.b] += complex_traits::scalar_t(1, 0);
				}
			}

			std::vector< double > lambda;
			vector_t a(e_);

			MKL_INT info = LAPACKE_zheev( LAPACK_COL_MAJOR, 'V', 'L', N, &e_[0], N, &lambda[0] );

			assert(info == 0);
			check(e_, lambda, a);
		}

		void check(vector_t const & e, std::vector< double > const & v, vector_t const & a)
		{
			for ( size_t k = 0; k != N; ++k )
			{
				for ( size_t i = 0; i != N; ++i )
				{
					complex_traits::scalar_t r = 0;
					for ( size_t j = 0; j != N; ++j )
					{
						r += e[k * N + j] * a[i * N + j];
					}
					assert(abs(e[k * N + i] * v[k] - r) < 1e-6);
				}
			}
		}
	};
}
