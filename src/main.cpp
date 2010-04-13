#include <vector>
#include <fstream>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <boost/utility.hpp>
#include <assert.h>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>

#define foreach BOOST_FOREACH

#include "utility/dump.h"
#include "algorithms/median.h"
#include "watermarking/embedding.h"

int main( int argc, char** argv )
{
    typedef watermarking::planar_graph< CGAL::Exact_predicates_inexact_constructions_kernel::Point_2 > graph_t;
    typedef std::auto_ptr< watermarking::embedding_impl< graph_t::vertex_t > > embedded_watermark_t;
    embedded_watermark_t ew = watermarking::embed( graph_t(), watermarking::message_t() ); 
}
