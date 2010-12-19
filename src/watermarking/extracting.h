#ifndef _EXTRACTING_H_
#define _EXTRACTING_H_

#include "common.h"

namespace watermarking
{
    // Предполагаем пока что V(old_graph) <--> V(graph), даже порядок совпадает
    message_t extract(  geometry::planar_graph_t const & old_graph,
                        geometry::planar_graph_t const & graph, 
                        std::vector< size_t > const & subdivision,
                        std::vector< analyser_ptr > const & analyser_vec,
                        int key, size_t chip_rate, size_t message_size );
}

#endif /* _EXTRACTING_H_ */
