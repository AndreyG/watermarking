#ifndef _EXTRACTING_H_
#define _EXTRACTING_H_

#include "../algorithms/util.h"
#include "../geometry/planar_graph.h"

namespace watermarking
{
    namespace
    {
        int sign( double r )
        {
            if ( r < 0 )
                return -1;
            else if ( r > 0 )
                return 1;
            else
                return 0;
        }
    }

    // Предполагаем пока что V(old_graph) <--> V(graph), даже порядок совпадает
    message_t extract(  geometry::planar_graph_t const & old_graph,
                        geometry::planar_graph_t const & graph, 
                        std::vector< size_t > const & subdivision,
                        std::vector< boost::shared_ptr< spectral_analyser > > const & analyser_vec,
                        int key, size_t chip_rate, size_t message_size )
    {
		const size_t N = subdivision.size();
        assert( old_graph.vertices_num() == N );
        assert( graph.vertices_num()     == N );

        using algorithm::make_max;
        
        size_t areas_num = 0;
        foreach ( size_t s, subdivision )
        {
            make_max( areas_num, s );
        }
        assert( areas_num == analyser_vec.size() - 1 );

        typedef geometry::planar_graph_t          	graph_t;
        typedef graph_t::vertex_t       			vertex_t;
        typedef std::vector< vertex_t >				vertices_t;

        std::vector< vertices_t > vertices( analyser_vec.size() );
        for ( size_t i = 0; i != N; ++i )
        {
            vertices[subdivision[i]].push_back( graph.vertex( i ) - old_graph.vertex( i ) );
        }

        message_t res( message_size );
        for ( size_t subarea = 0; subarea != vertices.size(); ++subarea )
        {
            std::vector< double > coeffs = analyser_vec[subarea]->get_coefficients( vertices[subarea] );
            srand( key );
            for ( size_t i = 0; i != message_size * chip_rate; )
            {
                double r = 0;
                for ( size_t j = 0; j != chip_rate; ++j, ++i )
                {
                    int p = ( rand() % 2 ) * 2 - 1;
                    r += p * coeffs[i];
                }
                res[i / chip_rate - 1] += sign( r );
            }
        }
        for ( size_t i = 0; i != message_size; ++i )
        {
            res[i] = (1 + sign(res[i])) / 2;
        }
        return res;
    }
}

#endif /* _EXTRACTING_H_ */
