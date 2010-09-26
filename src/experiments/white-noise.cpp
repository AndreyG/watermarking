typedef CGAL::Exact_predicates_inexact_constructions_kernel             Gt;
typedef CGAL::Exact_predicates_inexact_constructions_kernel::Point_2    point_t; 

typedef std::auto_ptr< spectral_analyser > analyser_ptr;

analyser_ptr analyse( std::vector< point_t > const & points )
{
    typedef CGAL::Delaunay_triangulation_2< Gt >                    trg_t;

    trg_t trg;
    foreach ( point_2 const & pt, vertices )
    {
        trg.insert( pt );
    }
    {
        util::stopwatch _("factorization");

        std::map< typename trg_t::Vertex_handle, size_t > trg_vertex_to_index;
        size_t i = 0;

        typedef typename trg_t::Finite_vertices_iterator vertices_iterator; 
        for ( vertices_iterator v = trg.finite_vertices_begin(); v != trg.finite_vertices_end(); ++v )
        {
            trg_vertex_to_index.insert( std::make_pair( v, i ) );
            vertices.push_back( v->point() );
            ++i;
        }

        typedef geometry::triangulation_graph< trg_t > incidence_graph;
        return analyser_ptr( new spectral_analyser( incidence_graph( trg, trg_vertex_to_index, true ) ) );
    }
}

std::vector< point_t > generate_noise( const size_t N )
{
    std::ifstream noise_conf("noise.conf");
                
    po::options_description desc;
    desc.add_options()
        ( "epsilon",   po::value< double >() )
    ;

    po::variables_map vm;       
    po::store( po::parse_config_file( conf, desc ), vm );

    std::vector< point_t > res( N );
    util::random_generator gen( vm["epsilon"].as< double >() );
    for ( size_t i = 0; i != N; ++i )
    {
        double a = ( 2 * M_PI * rand() ) / RAND_MAX;
        double r = gen();
        res[i] = point_t( cos( a ) * r, sin( a ) * r );
    }
    return res;
}

int main()
{
    std::vector< point_t > points = read_data();
    analyser_ptr analyser = analyse( points );
    std::vector< point_t > noise = generate_noise( points.size() );
    std::vector< point_t > coefficients = analyser->get_coefficients( noise );

    foreach ( point_t const & c, coefficients )
    {
        std::ofstream << c.x() << " " << c.y() << "\n";
    }
}
