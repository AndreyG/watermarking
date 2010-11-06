#include "stdafx.h"

#include "utility/dump.h"
#include "utility/stopwatch.h"

#include "data_reading.h"

#include "watermarking/embedding.h"
#include "watermarking/extracting.h"

typedef CGAL::Exact_predicates_inexact_constructions_kernel::Point_2    point_t; 
typedef watermarking::planar_graph< point_t >                           graph_t;
typedef std::auto_ptr< watermarking::embedding_impl< point_t > >        embedding_impl_ptr;

graph_t create_graph( const char * filepath, bool fix = true )
{
    std::ifstream in(filepath);
    
    graph_t graph;
    read_graph( graph, in );
    if (fix)
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
        ( "weight-type",  po::value<std::string>() )
        ( "use-edges",    po::value<bool>() )
        ( "max-subarea-size", po::value< size_t >() )
        ( "fill-matrix",  po::value<std::string>() )
    ;
    
    po::variables_map params;       
    po::store(po::parse_config_file(conf, desc), params);

    const auto weight_type        = geometry::WeightType::from_str( params["weight-type"].as< std::string >() );
    const bool use_edges          = params["use-edges"].as< bool >();
    const size_t max_subarea_size = params["max-subarea-size"].as< size_t >();
    const bool step_by_step       = params["step-by-step"].as< bool >();
    const watermarking::FILL_MATRIX_TYPE fill_matrix = 
        params["fill-matrix"].as< std::string >() == "fill_matrix_by_chen" ? 
            watermarking::CHEN : watermarking::OBUCHI;
    return watermarking::embed( graph, max_subarea_size, weight_type, use_edges, step_by_step, fill_matrix );
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

double angle(point_t const & u, point_t const & v, point_t const & w)
{
    assert(u != v);
    assert(w != v);
    assert(u != w);

    double x1 = u.x() - v.x();
    double y1 = u.y() - v.y();
    double x2 = w.x() - v.x();
    double y2 = w.y() - v.y();

    double c = (x1 * x2 + y1 * y2) / (sqrt(x1 * x1 + y1 * y1) * sqrt(x2 * x2 + y2 * y2));
    using algorithm::make_min;
    using algorithm::make_max;
    if (isnan(c))
    {
        std::cout << std::setprecision(32) << "(" << x1 << ", " << y1 << "); (" << x2 << ", " << y2 << ")" << std::endl;
        std::cout << "u = ";
        dump(std::cout, u) << "; v = ";
        dump(std::cout, v) << "; w = ";
        dump(std::cout, w) << std::endl;
        assert(false);
    }
    make_min(c, 1.0);
    make_max(c, -1.0);
    return acos(c);
}

bool loops_exist(graph_t const & graph)
{
    foreach (typename graph_t::edge_t const & edge, graph.edges)
    {
        if (edge.first == edge.second)
            return true;
    }
    return false;
}

bool has_duplicate_vertices(graph_t const & graph)
{
    std::set< point_t > vertices( graph.vertices.begin(), graph.vertices.end() );
    return vertices.size() < vertices_num( graph );
}

double abs(double x)
{
    if (x < 0)
        return -x;
    else
        return x;
}

tuple<double, size_t> angle_difference(graph_t const& g1, graph_t const& g2)
{
    assert(!loops_exist(g1));
    assert(!loops_exist(g2));
    assert(!has_duplicate_vertices(g1));
    assert(!has_duplicate_vertices(g2));

    const size_t N = vertices_num( g1 );
    std::vector< std::set< size_t > > edges( N );
    foreach (typename graph_t::edge_t const & e, g1.edges)
    {
        edges[e.first].insert(e.second);
        edges[e.second].insert(e.first);
    }
    double res = 0.0;
    size_t angles_num = 0;
    for (size_t v = 0; v != N; ++v)
    {
        if (edges[v].size() >= 2)
        {
            point_t const & p1 = g1.vertices[v];
            point_t const & p2 = g2.vertices[v];
            std::vector< size_t > e( edges[v].begin(), edges[v].end() );
            for (size_t i = 0; i + 1 != e.size(); ++i)
            {
                for (size_t j = i + 1; j != e.size(); ++j)
                {
                    ++angles_num;
                    assert(e[i] != v);
                    assert(e[j] != v);
                    double a1 = angle(g1.vertices[e[i]], p1, g1.vertices[e[j]]);
                    double a2 = angle(g2.vertices[e[i]], p2, g2.vertices[e[j]]);
                    assert(a1 >= 0);
                    assert(a1 < 3.15);
                    assert(a2 >= 0);
                    assert(a2 < 3.15);
                    res += abs(a1 - a2);
                    std::cout << std::setprecision(32) << a1 << " " << a2 << " " << (a1 - a2) << " " << abs(a1 - a2) << " " << res << "\n";
                }
            }
        }
    }
    return make_tuple(res, angles_num);
}

int main( int argc, char** argv )
{
    auto graph = create_graph(argv[1]);
    assert(!loops_exist(graph));
    assert(!has_duplicate_vertices(graph));
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

    size_t attempts_num = boost::lexical_cast< size_t >( argv[8] );
    size_t noises_num   = boost::lexical_cast< size_t >( argv[9] );
    assert((size_t) argc == 10 + noises_num);
    
    dump_graph(modified_graph, (const char *) argv[6]);
    {
        std::ofstream out(argv[7]);
        auto ad = angle_difference(rearranged_graph, modified_graph); 
        out << std::setprecision(32) << boost::get<0>(ad) / boost::get<1>(ad) 
            << "\n" << boost::get<0>(ad) 
            << "\n" << boost::get<1>(ad);
    }

    for (size_t i = 0; i != noises_num; ++i)
    {
        double noise = boost::lexical_cast< double >( argv[10 + i] );
        for (size_t j = 0; j != attempts_num; ++j)
        {
            std::stringstream outdirstream;
            outdirstream << argv[5] << std::string("/noise-") << std::string(argv[8 + i]) << std::string("/attempt-") << j << std::string("/");
            std::string out_dir = outdirstream.str();
            graph_t noised_graph = watermarking::add_noise( modified_graph, noise );
	        //dump_graph(noised_graph, (out_dir + "noised_graph.txt").c_str());
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
