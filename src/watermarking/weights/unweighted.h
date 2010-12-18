#include "../../spectral_analysis.h"
#include "../../geometry/triangulation_graph.h"

namespace watermarking
{
	struct unweighted_spectral_analyser : spectral_analyser_impl< real_traits >
	{
		typedef spectral_analyser_impl< real_traits > 	base_t;

		explicit unweighted_spectral_analyser( std::istream & in )
				: base_t( in )
		{}

		template< class Trg >
		explicit unweighted_spectral_analyser( geometry::triangulation_graph< Trg > const & g )
				: base_t( g )
		{
			typedef geometry::triangulation_graph< Trg > graph_t;

			for ( size_t i = 0; i != g.edges_num(); ++i )
			{
				typename graph_t::edge_t const & e = g.edge( i );

				e_[e.b * N + e.e] = -1.0;
				e_[e.b * N + e.b] += 1.0;
				e_[e.e * N + e.b] = -1.0;
				e_[e.e * N + e.e] += 1.0;
			}

			for ( size_t i = 0; i != g.vertices_num(); ++i )
			{
				const double d = e_[i * N + i];
				for ( size_t j = 0; j != g.vertices_num(); ++j )
				{
					e_[i * N + j] /= d;
				}
			}

			vector_t tmp(N);
			vector_t ans(N * N);

			MKL_INT info = LAPACKE_dgeev(LAPACK_COL_MAJOR, 'N', 'V', N, &e_[0], N, &tmp[0], &tmp[0], NULL, 0, &ans[0], N * N);
			assert(info == 0);
			e_.swap(ans);
		}
	};
}
