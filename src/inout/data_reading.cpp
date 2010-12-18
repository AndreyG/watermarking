#include "../stdafx.h"

#include "inout.h"

namespace inout
{
message_params_t read_message_params(const char * filepath)
{
    std::ifstream conf(filepath);
                
    namespace po = boost::program_options;

    po::options_description desc;
    desc.add_options()
        ( "message", po::value< std::string >() )
        ( "key",     po::value< int >() )
        ( "chip-rate", po::value< size_t >() )
        ( "alpha",   po::value< double >() )
    ;

    po::variables_map vm;       
    po::store( po::parse_config_file( conf, desc ), vm );

    message_params_t params;
    params.text = vm["message"].as< std::string >();
    params.key  = vm["key"].as< int >();
    params.chip_rate = vm["chip-rate"].as< size_t >();
    params.alpha = vm["alpha"].as< double >();

    return params;
}

template< class Stream >
geometry::point_t read_point( Stream & in  )
{
    double x, y;
    skip_char( in, '(' );
    in >> x;
    skip_char( in, ',' );
    in >> y;
    skip_char( in, ')' );   
	return geometry::point_t(x, y);
}

void read_graph( geometry::planar_graph_t & graph, std::istream & in )
{
    size_t n, m;
    in >> n >> m;
	for ( size_t i = 0; i != n; ++i )
	{
		graph.add_vertex(read_point(in));
	}
	for ( size_t i = 0; i != m; ++i )
	{
		geometry::planar_graph_t::edge_t e;
		in >> e.b >> e.e;
		graph.add_edge(e);
	}
}
}
