#include "../stdafx.h"

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_2.h>

#include "../spectral_analysis.h"
#include "../geometry/triangulation_graph.h"
#include "../utility/random_generator.h"
#include "../data_reading.h"

namespace po = boost::program_options;

typedef CGAL::Exact_predicates_inexact_constructions_kernel             Gt;
typedef CGAL::Exact_predicates_inexact_constructions_kernel::Point_2    point_t; 

typedef std::auto_ptr< watermarking::spectral_analyser > analyser_ptr;

analyser_ptr analyse( std::vector< point_t > const & points )
{
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
    typedef analyser_ptr::element_type analyser_t;
    boost::function< void (incidence_graph const &, analyser_t::matrix_t &) > fill_matrix =
        boost::bind( &watermarking::details::fill_matrix_by_chen< incidence_graph, typename analyser_t::matrix_t >, _1, _2 ); 
    analyser_t * analyser = new analyser_t( graph, fill_matrix );
    return analyser_ptr( analyser );
}

std::vector< point_t > generate_noise( po::variables_map const & vm, const size_t N )
{
    std::vector< point_t > res( N );
    util::random_generator gen( vm["epsilon"].as< double >() );
    for ( size_t i = 0; i != N; ++i )
    {
        double a = ( 2 * M_PI * rand() ) / RAND_MAX;
        double r = gen();
        res[i] = point_t( cos( a ) * r, sin( a ) * r );
    }
    double len_x = 0, len_y = 0;
    for ( size_t i = 0; i != N; ++i )
    {
        len_x += res[i].x() * res[i].x();
        len_y += res[i].y() * res[i].y();
    }
    len_x = sqrt(len_x), len_y = sqrt(len_y);
    for ( size_t i = 0; i != N; ++i )
    {
        res[i] = point_t(res[i].x() / len_x, res[i].y() / len_y);
    }
    return res;
}

std::vector< point_t > read_data( po::variables_map const & vm )
{
    std::vector< point_t > pts( vm["vertices-num"].as< size_t >() );
    std::ifstream in( vm["input-name"].as< std::string >().c_str() );
    size_t n, m;
    in >> n >> m;
    assert( n >= pts.size() );
    read_points( in, pts.size(), &pts[0] );
    std::set< point_t > point_set( pts.begin(), pts.end() );
    return std::vector< point_t >( point_set.begin(), point_set.end() );
}

int main()
{
    std::ifstream conf("white-noise-experiment.conf");

    po::options_description desc;
    desc.add_options()
        ( "input-name", po::value< std::string >() )
        ( "vertices-num", po::value< size_t >() )
        ( "epsilon",   po::value< double >() )
    ;

    po::variables_map vm;
    po::store( po::parse_config_file( conf, desc ), vm );
    
    std::vector< point_t > points = read_data( vm );
    analyser_ptr analyser = analyse( points );
    std::vector< point_t > noise = generate_noise( vm, points.size() );
    std::vector< point_t > coefficients = analyser->get_coefficients( noise );

    std::cout << coefficients.size() << "\n";
    foreach ( point_t const & c, coefficients )
    {
        std::cout << c.x() << " " << c.y() << "\n";
    }
}
