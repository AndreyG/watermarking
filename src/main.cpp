#include "stdafx.h"

#include "utility/dump.h"
#include "utility/stopwatch.h"

/*
#include "watermarking/embedding.h"
#include "watermarking/extracting.h"
#include "visualization/my_visualizer.h"
*/
#include "data_reading.h"

int main( int argc, char** argv )
{
/*
    namespace po = boost::program_options;

    po::variables_map params = read_params();

    std::ifstream in( params["input-data"].as< std::string >().c_str() );
    
    typedef CGAL::Exact_predicates_inexact_constructions_kernel::Point_2 point_t; 
    typedef watermarking::planar_graph< point_t > graph_t;

    graph_t graph;
    read_graph( graph, in );
    fix_graph( graph );
    
    typedef std::auto_ptr< watermarking::embedding_impl< graph_t::vertex_t > > embedded_watermark_t;
    bool weighted           = params["weighted"].as< bool >();
    bool use_edges          = params["use-edges"].as< bool >();
    size_t max_subarea_size = params["max-subarea-size"].as< size_t >();
    embedded_watermark_t ew = watermarking::embed( graph, max_subarea_size, weighted, use_edges, params["step-by-step"].as< bool >() );
                                                             
    std::ifstream conf("embedding.conf");
                
    po::options_description desc;
    desc.add_options()
        ( "message", po::value< std::string >() )
        ( "key",     po::value< int >() )
        ( "chip-rate", po::value< size_t >() )
        ( "alpha",   po::value< double >() )
    ;

    po::variables_map vm;       
    po::store( po::parse_config_file( conf, desc ), vm );

    std::string message_text = vm["message"].as< std::string >();
    std::cout << message_text << std::endl;
    
    watermarking::message_t message( message_text.size() * 7 );
    for ( size_t i = 0; i != message_text.size(); ++i )
    {
        unsigned char c = message_text[i];
        for ( size_t j = 0; j != 7; ++j )
        {
            message[i * 7 + j] = c % 2;
            std::cout << message[i * 7 + j];
            c /= 2;
        }
    }
    std::cout << std::endl;
    
    ew->modify_vertices( message,   vm["chip-rate"].as< size_t >(),
                                    vm["key"].as< int >(),
                                    vm["alpha"].as< double >() );
    graph_t modified_graph = ew->modified_graph();
    std::vector< std::vector< point_t > > old_coefficients = ew->coefficients_;
    ew.reset();
    for ( double radius = 0.1; radius <= 1.0; radius += 0.1 )
    {
        graph_t noised_graph = watermarking::add_noise( modified_graph, radius );
        watermarking::message_t ex_message = watermarking::extract( noised_graph, 
                                                                    max_subarea_size,
                                                                    weighted )->extract( vm["key"].as< int >(), vm["chip-rate"].as< size_t >(), old_coefficients, message.size() ); 
        std::cout << "Embedded message:  ";
        for ( size_t i = 0; i != message.size(); )
        {
            for ( size_t j = 0; j != 7; ++j, ++i )
                std::cout << message[i];
            std::cout << " ";
        }

        std::cout << "Extracted message: ";
        std::vector< unsigned char > mes;
        for ( size_t i = 0; i != ex_message.size(); )
        {
            unsigned char c = 0;
            int pow = 1;
            for ( size_t j = 0; j != 7; ++j, ++i, pow *= 2 )
            {
                std::cout << ex_message[i];
                c += ex_message[i] * pow;
            }
            std::cout << " ";
            mes.push_back( c );
        }
        std::cout << std::endl;
        std::copy( mes.begin(), mes.end(), std::ostream_iterator< char >( std::cout, "" ) );
        std::cout << std::endl;
    }

 
//    typedef my_visualizer< embedded_watermark_t::element_type > visualizer_t; 
//    visualizer_t v( ew.get() ); 
//    boost::format title("W: weighted = %1%, use-edges = %2%");
//    visualization::viewer< visualizer_t > viewer( &v, ( title % weighted % use_edges ).str().c_str(), &argc, argv );
*/
}
