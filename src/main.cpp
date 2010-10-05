#include "stdafx.h"

#include "utility/dump.h"
#include "utility/stopwatch.h"

#include "data_reading.h"

#include "watermarking/embedding.h"
#include "watermarking/extracting.h"

typedef CGAL::Exact_predicates_inexact_constructions_kernel::Point_2    point_t; 
typedef watermarking::planar_graph< point_t >                           graph_t;
typedef std::auto_ptr< watermarking::embedding_impl< point_t > >        embedding_impl_ptr;

graph_t create_graph( const char * filepath )
{
    std::ifstream in(filepath);
    
    graph_t graph;
    read_graph( graph, in );
    fix_graph( graph );

    return graph;
}

embedding_impl_ptr create_embedding( graph_t const & graph, const char * filepath )
{
    namespace po = boost::program_options;

    std::ifstream conf(filepath);
    po::options_description desc;
    desc.add_options()
        ( "step-by-step", po::value<bool>() )
        ( "weighted",     po::value<bool>() )
        ( "use-edges",    po::value<bool>() )
        ( "max-subarea-size", po::value< size_t >() )
        ( "fill-matrix",  po::value<std::string>() )
    ;
    
    po::variables_map params;       
    po::store(po::parse_config_file(conf, desc), params);

    const bool weighted           = params["weighted"].as< bool >();
    const bool use_edges          = params["use-edges"].as< bool >();
    const size_t max_subarea_size = params["max-subarea-size"].as< size_t >();
    const bool step_by_step       = params["step-by-step"].as< bool >();
    const watermarking::FILL_MATRIX_TYPE fill_matrix = 
        params["fill-matrix"].as< std::string >() == "fill_matrix_by_chen" ? 
            watermarking::CHEN : watermarking::OBUCHI;
    return watermarking::embed( graph, max_subarea_size, weighted, use_edges, step_by_step, fill_matrix );
}

namespace
{
    watermarking::message_t encode( std::string const & text )
    {
        watermarking::message_t message;
        message.resize(text.size() * 7);
        for ( int i = 0; i != (int) text.size(); ++i )
        {
            unsigned char c = text[i];
            for ( int j = 6; j >= 0; --j )
            {
                message[i * 7 + j] = c % 2;
                c /= 2;
            }
        }
        return message;
    }

    std::string decode( watermarking::message_t const & message )
    {
        assert( message.size() % 7 == 0 );
        std::vector< char > text( message.size() / 7 );
        for ( size_t i = 0; i != text.size(); ++i )
        {
            text[i] = 0;
            unsigned char power = 1;
            for ( int j = 6; j >= 0; --j )
            {
                text[i] += power * message[i * 7 + j];
                power *= 2;
            }
        }
        return std::string( text.begin(), text.end() );
    }

    template< class Stream >
    void write_encoded_message( Stream & out, watermarking::message_t const & message )
    {
        assert(message.size() % 7 == 0);
        for ( size_t i = 0; i != message.size(); )
        {
            for ( size_t j = 0; j != 7; ++j, ++i )
                out << message[i];
            out << " ";
        }
        out << std::endl;
    }
}

void dump_graph(graph_t const & graph, const char * filepath)
{
    std::ofstream out(filepath);
    dump_graph(graph, out);
}

int main( int argc, char** argv )
{
    auto graph  = create_graph( argv[1] );
    embedding_impl_ptr ew;
    {
        auto dumppath = (std::string(argv[3]) + "/factorization.params"); 
        if ( std::string(argv[2]) == std::string("dump") )
        {
            std::ifstream in( dumppath.c_str() ); 
            ew.reset( new watermarking::embedding_impl< point_t >( in ) );
        }
        else
        {
            auto inputpath = std::string(argv[2]); 
            ew = create_embedding( graph, inputpath.c_str() );
            std::ofstream out( dumppath.c_str() ); 
            ew->dump( out );
        }
    }
    auto message_params = read_message_params(argv[4]);
    auto message = encode( message_params.text );
    
    ew->modify_vertices( message, message_params.chip_rate, message_params.key, message_params.alpha );

    graph_t const & rearranged_graph = ew->rearranged_graph();
    graph_t modified_graph = ew->modified_graph();
    
    std::vector< watermarking::analyser_ptr > analyser_vec = ew->get_analysers();
    std::vector< size_t > const & subdivision = ew->get_subdivision();

    size_t attempts_num = boost::lexical_cast< size_t >( argv[6] );
    size_t noises_num   = boost::lexical_cast< size_t >( argv[7] );
    assert((size_t) argc == 8 + noises_num);
    
    dump_graph(modified_graph, (std::string(argv[5]) + "/modified_graph.txt").c_str());

    for (size_t i = 0; i != noises_num; ++i)
    {
        double noise = boost::lexical_cast< double >( argv[8 + i] );
        for (size_t j = 0; j != attempts_num; ++j)
        {
            std::stringstream outdirstream;
            outdirstream << argv[5] << std::string("/noise-") << std::string(argv[8 + i]) << std::string("/attempt-") << j << std::string("/");
            std::string out_dir = outdirstream.str();
            graph_t noised_graph = watermarking::add_noise( modified_graph, noise );
	        dump_graph(noised_graph, (out_dir + "noised_graph.txt").c_str());
	        watermarking::message_t ex_message = watermarking::extract( rearranged_graph, noised_graph, subdivision, 
			                                    					    analyser_vec, message_params.key,
                                                                        message_params.chip_rate, message.size() ); 
	        std::ofstream out((out_dir + "message.txt").c_str());
	        out << "Embedded message:  ";
            write_encoded_message( out, message );
            out << decode( message ) << std::endl;
            out << "Extracted message: ";
            write_encoded_message( out, ex_message );
            out << decode( ex_message ) << std::endl;
        }
    }
}
