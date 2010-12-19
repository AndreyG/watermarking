#ifndef _COMMON_H_
#define _COMMON_H_

#include "spectral_analysis.h"
#include "../geometry/planar_graph.h"

namespace watermarking
{
	typedef std::vector< int > 						message_t;
	typedef boost::shared_ptr< spectral_analyser > 	analyser_ptr;
}

#endif /*_COMMON_H_*/
