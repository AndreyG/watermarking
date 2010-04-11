#ifndef _SPECTRAL_ANALYSIS_H_
#define _SPECTRAL_ANALYSIS_H_

#include <tnt/tnt_array2d.h>

namespace watermarking
{
    typedef TNT::Array2D< double >  matrix_t;

    template< class Graph >
    matrix_t spectral_analysis( Graph const & graph )
    {
    }
}

#endif /* _SPECTRAL_ANALYSIS_H_ */
