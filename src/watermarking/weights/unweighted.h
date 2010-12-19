#include "../spectral_analysis.h"
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

			vector_t lambda_real(N), lambda_imag(N);
			vector_t vectors_left(N * N), vectors_right(N * N);

			MKL_INT info = LAPACKE_dgeev(	LAPACK_COL_MAJOR, 'N', 'V', N, &e_[0], N, 
											&lambda_real[0], &lambda_imag[0], 
											&vectors_left[0], N, &vectors_right[0], N);
			check(vectors_right, lambda_real, lambda_imag, e_);
			assert(info == 0);
			e_.swap(vectors_right);
		}

		void check(vector_t const & e, vector_t const & vr, vector_t const & vi, vector_t const & a)
		{
			foreach ( double d, vi )
				assert( d == 0 );

			for ( size_t k = 0; k != N; ++k )
			{
				for ( size_t i = 0; i != N; ++i )
				{
					double r = 0;
					for ( size_t j = 0; j != N; ++j )
					{
						r += e[k * N + j] * a[i * N + j];
					}
					assert(abs(e[k * N + i] * vr[k] - r) < 1e-6);
				}
			}
		}
	};
}
