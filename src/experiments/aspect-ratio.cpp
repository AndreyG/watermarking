#include "../stdafx.h"

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_2.h>

#include "../spectral_analysis.h"
#include "../geometry/triangulation_graph.h"

namespace po = boost::program_options;

typedef CGAL::Exact_predicates_inexact_constructions_kernel             Gt;
typedef CGAL::Exact_predicates_inexact_constructions_kernel::Point_2    point_t; 

std::vector< point_t > generate_random_points( const size_t N, const double a, const double b )
{
    std::vector< point_t > pts( N );
    for ( size_t i = 0; i != N; ++i )
    {
        pts[i] =    point_t( 
                        a * ( ( rand() * 1.0 ) / RAND_MAX ),
                        b * ( ( rand() * 1.0 ) / RAND_MAX )
                    );
    }
    return pts;
}

int main()
{
    std::ifstream conf("aspect-ratio.conf");

    po::options_description desc;
    desc.add_options()
        ( "a", po::value< double >() )
        ( "b", po::value< double >() )
        ( "n", po::value< size_t >() )
    ;

    po::variables_map vm;
    po::store( po::parse_config_file( conf, desc ), vm );

    const size_t n = vm["n"].as< size_t >();
    const double a = vm["a"].as< double >();
    const double b = vm["b"].as< double >();
    std::vector< point_t > points = generate_random_points( n, a, b );

    typedef CGAL::Delaunay_triangulation_2< Gt >                    trg_t;

    trg_t trg;
    foreach ( point_t const & pt, points )
    {
        trg.insert( pt );
    }
    
    util::stopwatch _("factorization");

    std::map< typename trg_t::Vertex_handle, size_t > trg_vertex_to_index;
    size_t i = 0;

    typedef typename trg_t::Finite_vertices_iterator vertices_iterator; 
    for ( vertices_iterator v = trg.finite_vertices_begin(); v != trg.finite_vertices_end(); ++v )
    {
        trg_vertex_to_index.insert( std::make_pair( v, i ) );
        ++i;
    }

    typedef geometry::triangulation_graph< trg_t > incidence_graph;
    incidence_graph graph( trg, trg_vertex_to_index, true );
    typedef watermarking::spectral_analyser analyser_t;
    boost::function< void (incidence_graph const &, analyser_t::matrix_t &) > fill_matrix =
        boost::bind( &watermarking::details::fill_matrix_by_chen< incidence_graph, typename analyser_t::matrix_t >, _1, _2 ); 
    analyser_t analyser( graph, fill_matrix );

    double lambda[3];
    for ( size_t i = 0; i != 3; ++i )
    {
        lambda[i] = analyser.get_lambda(i);
        std::cout << "lambda_" << i << " = " << lambda[i] << "\n";
    }
    std::cout << "aspect ratio = " << a / b << std::endl;
    std::cout << "lambda ratio = " << lambda[1] / lambda[2] << std::endl;
}
