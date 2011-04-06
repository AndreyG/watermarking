#include "stdafx.h"

#include "utility/stopwatch.h"

#include "inout/inout.h"

#include "watermarking/embedding.h"
#include "watermarking/extracting.h"
#include "algorithms/util.h"

typedef geometry::planar_graph_t                         graph_t;
typedef std::auto_ptr< watermarking::embedding_impl >    embedding_impl_ptr;

void fix_graph( graph_t & graph );

graph_t create_graph( const char * filepath, bool fix = true )
{
    std::ifstream in(filepath);
    
    graph_t graph;
    inout::read_graph( graph, in );
    if (fix)
        fix_graph( graph );

    return graph;
}

embedding_impl_ptr create_embedding( graph_t const & graph, std::string const & factorization_conf )
{
    namespace po = boost::program_options;

    po::options_description desc;
    desc.add_options()
        ( "step-by-step", po::value<bool>() )
        ( "weight-type",  po::value<std::string>() )
        ( "use-edges",    po::value<bool>() )
        ( "max-subarea-size", po::value< size_t >() )
    ;
    
    po::variables_map params;       
    std::ifstream conf(factorization_conf);
    po::store(po::parse_config_file(conf, desc), params);

    const auto weight_type        = watermarking::WeightType::from_str(params["weight-type"].as< std::string >());
    const bool use_edges          = params["use-edges"].as< bool >();
    const size_t max_subarea_size = params["max-subarea-size"].as< size_t >();
    const bool step_by_step       = params["step-by-step"].as< bool >();
    return embedding_impl_ptr(new watermarking::embedding_impl(graph, max_subarea_size, weight_type, use_edges, step_by_step));
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
    inout::write_graph(graph, out);
}

double angle(geometry::point_t const & u, geometry::point_t const & v, geometry::point_t const & w)
{
    assert(u != v);
    assert(w != v);
    assert(u != w);

	auto v1 = u - v;
	auto v2 = w - v;

    double c = (v1 * v2) / (sqrt(mod(v1)) * sqrt(mod(v2)));
    using algorithm::make_min;
    using algorithm::make_max;
	assert(!isnan(c));
    make_min(c, 1.0);
    make_max(c, -1.0);
    return acos(c);
}

bool loops_exist(graph_t const & graph)
{
    for ( auto edge = graph.edges_begin(); edge != graph.edges_end(); ++edge )
    {
		if ( edge->b == edge->e )
            return true;
    }
    return false;
}

bool has_duplicate_vertices(graph_t const & graph)
{
    std::set< geometry::point_t > vertices( graph.vertices_begin(), graph.vertices_end() );
    return vertices.size() < graph.vertices_num();
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

    const size_t N = g1.vertices_num();
    std::vector< std::set< size_t > > edges( N );
    for (auto e = g1.edges_begin(); e != g1.edges_end(); ++e)
    {
        edges[e->b].insert(e->e);
        edges[e->e].insert(e->b);
    }
    double res = 0.0;
    size_t angles_num = 0;
    for (size_t v = 0; v != N; ++v)
    {
        if (edges[v].size() >= 2)
        {
			graph_t::vertex_t const & p1 = g1.vertex(v);
			graph_t::vertex_t const & p2 = g2.vertex(v);
            std::vector< size_t > e( edges[v].begin(), edges[v].end() );
            for (size_t i = 0; i + 1 != e.size(); ++i)
            {
                for (size_t j = i + 1; j != e.size(); ++j)
                {
                    ++angles_num;
                    assert(e[i] != v);
                    assert(e[j] != v);
                    double a1 = angle(g1.vertex(e[i]), p1, g1.vertex(e[j]));
                    double a2 = angle(g2.vertex(e[i]), p2, g2.vertex(e[j]));
                    assert(a1 >= 0);
                    assert(a1 < 3.15);
                    assert(a2 >= 0);
                    assert(a2 < 3.15);
                    res += abs(a1 - a2);
                }
            }
        }
    }
    return make_tuple(res, angles_num);
}

int main( int argc, char** argv )
{
    using inout::config_t;
    config_t config(argc, argv);

    embedding_impl_ptr ew;
    {
        if (config.dump_exists)
        {
            std::ifstream in(config.dump_path.c_str()); 
            ew.reset(new watermarking::embedding_impl(in));
        }
        else
        {
            auto graph = create_graph(config.input_graph.c_str());
            assert(!loops_exist(graph));
            assert(!has_duplicate_vertices(graph));
            
            ew = create_embedding(graph, config.factorization);
            std::ofstream out(config.dump_path.c_str()); 
            ew->dump(out);
        }
    }
    auto message_params = inout::read_message_params(config.embedding.c_str());
    auto message = encode(message_params.text);
    
    ew->modify_vertices(message, message_params.chip_rate, message_params.key, message_params.alpha);

    graph_t const & rearranged_graph = ew->rearranged_graph();
    graph_t modified_graph = ew->modified_graph();
    
    dump_graph(modified_graph, config.watermarked_graph.c_str());
    {
        std::ofstream out(config.statistics_file.c_str());
        auto ad = angle_difference(rearranged_graph, modified_graph); 
        out << std::setprecision(32) << boost::get<0>(ad) / boost::get<1>(ad) 
            << "\n" << boost::get<0>(ad) 
            << "\n" << boost::get<1>(ad);
    }

    auto analyser_vec = ew->get_analysers();
    std::vector< size_t > const & subdivision = ew->get_subdivision();

    for (double noise = config.noise_lower_bound; noise <= config.noise_upper_bound; noise += config.noise_step)
    {
        for (size_t j = 0; j != config.attempts_num; ++j)
        {
            std::stringstream outdirstream;
            outdirstream    << config.result_dir 
                            << "/noise-" << std::setprecision(2) << noise
                            << "/attempt-" << j << "/";
            std::string out_dir = outdirstream.str();

            graph_t noised_graph = geometry::add_noise(modified_graph, noise);
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
