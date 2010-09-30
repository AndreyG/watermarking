#ifndef _EXTRACTING_H_
#define _EXTRACTING_H_

#include "common.h"

namespace watermarking
{
    template< class Point >
    message_t extract(  planar_graph< Point > const & old_graph,
                        planar_graph< Point > const & graph, 
                        std::vector< analyser_ptr > const & analyser_vec,
                        int key, size_t chip_rate, size_t message_size )
    {
        return message_t();
    }
}

#endif /* _EXTRACTING_H_ */
