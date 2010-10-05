#ifndef _EXTRACTING_H_
#define _EXTRACTING_H_

#include "../algorithms/util.h"
#include "common.h"

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
    template< class Point >
    message_t extract(  planar_graph< Point > const & old_graph,
                        planar_graph< Point > const & graph, 
                        std::vector< size_t > const & subdivision,
                        std::vector< analyser_ptr > const & analyser_vec,
                        int key, size_t chip_rate, size_t message_size )
    {
        using algorithm::make_max;
        
        size_t areas_num = 0;
        foreach ( size_t s, subdivision )
        {
            make_max( areas_num, s );
        }
        assert( areas_num == analyser_vec.size() - 1 );

        std::vector< std::vector< Point > > vertices( analyser_vec.size() );
        assert( vertices_num( old_graph ) == subdivision.size() );
        assert( vertices_num( graph )     == subdivision.size() );
        for ( size_t i = 0; i != subdivision.size(); ++i )
        {
            Point const & old_vertex    = old_graph.vertices[i];
            Point const & vertex        = graph.vertices[i];
            vertices[subdivision[i]].push_back( Point( vertex.x() - old_vertex.x(), vertex.y() - old_vertex.y() ) );
        }

        message_t res( message_size );
        for ( size_t subarea = 0; subarea != vertices.size(); ++subarea )
        {
            std::vector< Point > coeffs = analyser_vec[subarea]->get_coefficients( vertices[subarea] );
            srand( key );
            for ( size_t i = 0; i != message_size * chip_rate; )
            {
                double r = 0;
                for ( size_t j = 0; j != chip_rate; ++j, ++i )
                {
                    int p = ( rand() % 2 ) * 2 - 1;
                    r += p * ( coeffs[i].x() + coeffs[i].y() );
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
