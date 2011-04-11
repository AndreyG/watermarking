#ifndef _INOUT_H_
#define _INOUT_H_

#include "../geometry/planar_graph.h"

namespace inout
{
	void read_graph( geometry::planar_graph_t & graph, std::istream & in );

    struct config_t
    {
        config_t(int argc, char** argv);

        bool dump_exists;
        std::string dump_path;

        std::string input_graph;
        std::string factorization;
        std::string embedding;
        
        std::string result_dir;
        std::string watermarked_graph;
        std::string statistics_file;
        
        size_t attempts_num;

        double noise_lower_bound, noise_upper_bound, noise_step;
    };
	
    struct factorization_params_t
    {
        explicit factorization_params_t(std::string const & filepath);

        std::string weight_type;
        size_t max_subarea_size;
        bool use_edges;
        bool step_by_step;
    };

	struct message_params_t
	{
        explicit message_params_t(std::string const & filepath);

    	std::string text;
    	size_t      chip_rate;
    	int         key;
    	double      alpha;
	};

	template< class Stream >
	void skip_char( Stream & in, char barrier )
	{
		while ( in )
		{
			char c;
			in >> c;
			if ( c == barrier )
				break;
		}
	}

	void write_graph( geometry::planar_graph_t const & graph, std::ostream & out );
}

#endif /*_INOUT_H_*/
