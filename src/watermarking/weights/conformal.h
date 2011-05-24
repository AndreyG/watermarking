#include "../spectral_analysis.h"
#include "../../geometry/triangulation_graph.h"
#include "../../geometry/trigonometry.h"
#include "../../utility/stopwatch.h"
#include "../../utility/debug_stream.h"
#include "../../visualization/triangulation_graph_viewer.h"

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
			util::stopwatch _("conformal_spectral_analyser::ctor");

			fill_matrix(g);

			std::vector<double> lambda(N);
			vector_t a(e_);
            vector_t a_copy(a);

            MKL_INT eigenvalues_num = 0;

			{
				util::stopwatch _("calculating eigenvectors");

                std::vector<MKL_INT> isuppz(N);
                MKL_INT il = 0, iu = 0;

				//MKL_INT info = LAPACKE_zheev( LAPACK_COL_MAJOR, 'V', 'L', N, &e_[0], N, &lambda[0] );
                MKL_INT info = LAPACKE_zheevr(  LAPACK_COL_MAJOR, 'V', 'V', 'L', N, &a[0], N, 
                                                0.0, 1e6, il, iu, 1e-7,
                                                &eigenvalues_num, &lambda[0], &e_[0], N, &isuppz[0] );

                if (info > 0)
                {
                    triangulation_graph_viewer_t<Trg, complex_traits::scalar_t> viewer(&g, a);
                    visualization::vis_system::run_viewer(&viewer);

				    throw std::exception();
                }
			}

            K = eigenvalues_num;

			//check(e_, lambda, a_copy);
		}

	private:

		double safe( double x ) const
		{
			assert( x != std::numeric_limits< double >::quiet_NaN() );
			assert( x != std::numeric_limits< double >::signaling_NaN() );
			if ( abs(x) == std::numeric_limits< double >::max() )
				return 0;
			if ( abs(x) == std::numeric_limits< double >::infinity() )
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

				size_t b = edge.b, e = edge.e;

				using geometry::ctg;

                size_t idx = (b < e) ? (b * N + e) : (e * N + b);

                if (b < e)
                {
				    e_[idx] = -safe(ctg(g.vertex(b), g.vertex(edge.left), g.vertex(e)));
				
				    if (g.is_valid(edge.right))
					    e_[idx] -= safe(ctg(g.vertex(b), g.vertex(edge.right), g.vertex(e)));

				    e_[b * N + b] -= e_[idx];
                    e_[e * N + e] -= e_[idx];

				    if (!g.is_valid(edge.right))
				    {
                        // важно, что мы используем COL_MAJOR_ORDER
                        // т.е., e_[e * N + b] = e_[b][e] бьет из b в е.
                        complex_traits::scalar_t imag_unit(0, 1);
                        if (b < e)
				            e_[idx] += imag_unit;
                        else
                            e_[idx] -= imag_unit;
                    }
                }
			}
		}
 
		void check(vector_t const & e, std::vector< double > const & v, vector_t const & a) const
		{
			util::stopwatch _("eigenvectors checking");

            // для каждого собственного вектора
			for ( size_t k = 0; k != K; ++k )
			{
                // для каждой строки
				for ( size_t i = 0; i != N; ++i )
				{
					complex_traits::scalar_t r = 0.0;
					for ( size_t j = 0; j != N; ++j )
					{
                        complex_traits::scalar_t a_ij = (i < j) ? a[i * N + j] : a[j * N + i];
                        if (i < j)
                            a_ij = complex_traits::scalar_t(a_ij.real(), -a_ij.imag());
                        r += e[k * N + j] * a_ij;
					}
                    if (!(abs(e[k * N + i] * v[k] - r) < 1e-3))
                        util::debug_stream() << v[k] << " " << r << " " << e[k * N + i] * v[k] 
                                             << abs(e[k * N + i] * v[k] - r);
					assert(abs(e[k * N + i] * v[k] - r) < 1e-3);
				}
			}
		}
	};
}
