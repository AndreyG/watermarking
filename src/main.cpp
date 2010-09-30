#include "stdafx.h"

#include "utility/dump.h"
#include "utility/stopwatch.h"

#include "watermarking/embedding.h"
#include "watermarking/extracting.h"
//#include "visualization/my_visualizer.h"

#include "data_reading.h"

typedef CGAL::Exact_predicates_inexact_constructions_kernel::Point_2    point_t; 
typedef watermarking::planar_graph< point_t >                           graph_t;
typedef std::auto_ptr< watermarking::embedding_impl< point_t > >        embedding_impl_ptr;

graph_t create_graph( std::string const & filepath )
{
    std::ifstream in( filepath.c_str() );
    
    graph_t graph;
    read_graph( graph, in );
    fix_graph( graph );

    return graph;
}

embedding_impl_ptr create_embedding( graph_t const & graph )
{
    namespace po = boost::program_options;

    std::ifstream conf("conf/common.conf");
    po::options_description desc;
    desc.add_options()
        ( "step-by-step", po::value<bool>() )
        ( "weighted",     po::value<bool>() )
        ( "use-edges",    po::value<bool>() )
        ( "max-subarea-size", po::value< size_t >() )
    ;
    
    po::variables_map params;       
    po::store(po::parse_config_file(conf, desc), params);

    const bool weighted           = params["weighted"].as< bool >();
    const bool use_edges          = params["use-edges"].as< bool >();
    const size_t max_subarea_size = params["max-subarea-size"].as< size_t >();
    const bool step_by_step       = params["step-by-step"].as< bool >();
    return watermarking::embed( graph, max_subarea_size, weighted, use_edges, step_by_step );
}

namespace
{
    watermarking::message_t encode( std::string const & text )
    {
        watermarking::message_t message( text.size() * 7 );
        for ( size_t i = 0; i != text.size(); ++i )
        {
            unsigned char c = text[i];
            for ( size_t j = 0; j != 7; ++j )
            {
                message[i * 7 + 6 - j] = c % 2;
                c /= 2;
            }
        }
        return message;
    }

    std::string decode( watermarking::message_t const & message )
    {
        assert( message.size() % 7 == 0 );
        std::vector< unsigned char > text( message.size() / 7 );
        for ( size_t i = 0; i != text.size(); ++i )
        {
            text[i] = 0;
            unsigned char power = 1;
            for ( size_t j = 0; j != 7; ++j )
            {
                text[i] += power * message[i * 7 + 6 - j];
                power *= 2;
            }
        }
        return std::string( text.begin(), text.end() );
    }
}


int main( int argc, char** argv )
{
    assert( argc >= 2 );
    auto graph  = create_graph( argv[1] );
    auto ew     = create_embedding( graph );

    auto message_params = read_message_params();
    auto message = encode( message_params.text );
    
    ew->modify_vertices( message, message_params.chip_rate, message_params.key, message_params.alpha );

    graph_t modified_graph = ew->modified_graph();
    
    std::vector< watermarking::analyser_ptr > analyser_vec = ew->get_analysers();
    for ( double radius = 0.0; radius <= 1.0; radius += 0.1 )
    {
        graph_t noised_graph = watermarking::add_noise( modified_graph, radius );
        watermarking::message_t ex_message = watermarking::extract( graph, noised_graph, analyser_vec, message_params.key,
                                                                    message_params.chip_rate, message.size() ); 
        std::cout << "Embedded message:  ";
        for ( size_t i = 0; i != message.size(); ++i )
        {
            for ( size_t j = 0; j != 7; ++j, ++i )
                std::cout << message[i];
            std::cout << " ";
        }
        std::cout << std::endl;

        std::cout << "Extracted message: ";
        for ( size_t i = 0; i != ex_message.size(); )
        {
            for ( size_t j = 0; j != 7; ++j, ++i )
                std::cout << ex_message[i];
            std::cout << " ";
        }
        std::cout << std::endl;
        std::cout << decode( ex_message ) << std::endl;
    }

 
//    typedef my_visualizer< embedded_watermark_t::element_type > visualizer_t; 
//    visualizer_t v( ew.get() ); 
//    boost::format title("W: weighted = %1%, use-edges = %2%");
//    visualization::viewer< visualizer_t > viewer( &v, ( title % weighted % use_edges ).str().c_str(), &argc, argv );
}
