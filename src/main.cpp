#include "stdafx.h"

#include "inout/inout.h"

#include "watermarking/embedding.h"
#include "watermarking/extracting.h"

#include "statistics.h"

#include "visualization/qtviewer.h"
#include "visualization/planar_graph_viewer.h"
#include "visualization/graph_diff_viewer.h"

typedef geometry::planar_graph_t                         graph_t;
typedef std::auto_ptr< watermarking::embedding_impl >    embedding_impl_ptr;

graph_t create_graph( std::string const & filepath )
{
    std::ifstream in(filepath.c_str());
    
    graph_t graph;
    inout::read_graph( graph, in );

    return graph;
}

embedding_impl_ptr create_embedding( graph_t const & graph, std::string const & factorization_conf )
{
    inout::factorization_params_t params(factorization_conf);

    const auto weight_type = watermarking::WeightType::from_str(params.weight_type);
    return embedding_impl_ptr(new watermarking::embedding_impl(graph, params.max_subarea_size, weight_type, params.use_edges, params.step_by_step));
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

void dump_graph(graph_t const & graph, std::string const & filepath)
{
    std::ofstream out(filepath.c_str());
    inout::write_graph(graph, out);
}

bool loops_exist(graph_t const &);
bool has_duplicate_vertices(graph_t const &);

int main( int argc, char** argv )
{
    vis_system::init(argc, argv);

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
            auto graph = create_graph(config.input_graph);
            assert(!loops_exist(graph));
            assert(!has_duplicate_vertices(graph));

            /*
            planar_graph_viewer_t viewer(&graph);
            vis_system::run_viewer(&viewer);
            */

            ew = create_embedding(graph, config.factorization);
            std::ofstream out(config.dump_path.c_str()); 
            ew->dump(out);
        }
    }
    inout::message_params_t message_params(config.embedding);
    auto message = encode(message_params.text);
    
    ew->modify_vertices(message, message_params.chip_rate, message_params.key, message_params.alpha);

    graph_t const & rearranged_graph = ew->rearranged_graph();
    graph_t modified_graph = ew->modified_graph();
    
    dump_graph(modified_graph, config.watermarked_graph);
    {
        util::stopwatch _("dumping statistics");

        std::ofstream out(config.statistics_file.c_str());
        auto ad = angle_difference(rearranged_graph, modified_graph); 
        out << std::setprecision(32) << boost::get<0>(ad) / boost::get<1>(ad) 
            << "\n" << boost::get<0>(ad) 
            << "\n" << boost::get<1>(ad);
    }

    std::vector< watermarking::analyser_ptr > const & analyser_vec = ew->get_analysers();
    std::vector< size_t > const & subdivision = ew->get_subdivision();

    for (double noise = config.noise_lower_bound; noise <= config.noise_upper_bound; noise += config.noise_step)
    {
        util::stopwatch _(boost::format("noise: %.3f") % noise);
        for (size_t j = 0; j != config.attempts_num; ++j)
        {
            //util::stopwatch _("attempt: " + boost::lexical_cast< std::string >(j));

            std::string out_dir = (boost::format("%s/noise-%.3f/attempt-%d/") % config.result_dir % noise % j).str();

            graph_t noised_graph = geometry::add_noise(modified_graph, noise);

            graph_diff_viewer_t viewer(&rearranged_graph, &modified_graph, &noised_graph);
            vis_system::run_viewer(&viewer);

	        watermarking::message_t ex_message = watermarking::extract( rearranged_graph, noised_graph, subdivision, 
			                                    					    analyser_vec, message_params.key,
                                                                        message_params.chip_rate, message.size() ); 
	        std::ofstream out((out_dir + "message.txt").c_str());
	        out << "Embedded message:  ";
            write_encoded_message(out, message);
            out << decode(message) << std::endl;
            out << "Extracted message: ";
            write_encoded_message(out, ex_message);
            out << decode(ex_message) << std::endl;
        }
    }
}
