#include "../stdafx.h"

#include "inout.h"

namespace inout
{

namespace
{
    template<typename T>
    void read(boost::program_options::variables_map const & vm, T & t, const char * name)
    {
        t = vm[name].as<T>();
    }
}

config_t::config_t(int argc, char** argv)
{
    namespace po = boost::program_options;

    po::options_description desc;
    desc.add_options()
        ("input-graph",         po::value< std::string >())
        ("dump-exists",         po::value< std::string >())
        ("factorization-dump",  po::value< std::string >())
        ("factorization",       po::value< std::string >())
        ("embedding",           po::value< std::string >())
        ("result-dir",          po::value< std::string >())
        ("watermarked-graph",   po::value< std::string >())
        ("statistics-file",     po::value< std::string >())
        ("noise-lower-bound",   po::value< double >())
        ("noise-upper-bound",   po::value< double >())
        ("noise-step",          po::value< double >())
    ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    dump_exists = vm["dump-exists"].as< std::string >() == "true";
    read(vm, dump_path,         "factorization-dump");
    read(vm, input_graph,       "input-graph");
    read(vm, factorization,     "factorization");
    read(vm, embedding,         "embedding");

    read(vm, result_dir,        "result-dir");
    read(vm, watermarked_graph, "watermarked_graph");
    read(vm, statistics_file,   "statistics-file");

    read(vm, noise_lower_bound, "noise_lower_bound");
    read(vm, noise_upper_bound, "noise_upper_bound");
    read(vm, noise_step,        "noise_step");
}

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
