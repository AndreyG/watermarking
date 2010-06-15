#include <vector>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <boost/utility.hpp>
#include <assert.h>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/type_traits/is_same.hpp>

#define foreach BOOST_FOREACH

#include "utility/dump.h"
#include "utility/stopwatch.h"

#include "watermarking/embedding.h"
#include "visualization/my_visualizer.h"

template< class Stream, class Point >
Stream & read_points( Stream & in, std::vector< Point > & out )
{
    double middle_x = 0, middle_y = 0;
    while ( in )
    {
        double x, y;
        util::skip_char( in, '(' );
        in >> x;
        util::skip_char( in, ',' );
        in >> y;
        util::skip_char( in, ')' );   
        middle_x += x;
        middle_y += y;
        out.push_back( Point( x, y ) );
        if ( out.size() == 100000 )
			break;
    }
    middle_x /= out.size();
    middle_y /= out.size();
    for ( typename std::vector< Point >::iterator it = out.begin(); it != out.end(); ++it )
        *it = Point( it->x() - middle_x, it->y() - middle_y );
    std::sort( out.begin(), out.end() );
    out.erase( std::unique( out.begin(), out.end() ), out.end() );
    return in;
}

int main( int argc, char** argv )
{
    assert( argc == 2 );
    std::ifstream in( argv[1] );
    
    typedef CGAL::Exact_predicates_inexact_constructions_kernel::Point_2 point_t; 
    typedef watermarking::planar_graph< point_t > graph_t;

    graph_t graph;
    read_points( in, graph.vertices );
    
    typedef std::auto_ptr< watermarking::embedding_impl< graph_t::vertex_t > > embedded_watermark_t;
    std::string message_text( "Very long message" );
    watermarking::message_t message( message_text.begin(), message_text.end() );
    embedded_watermark_t ew = watermarking::embed( graph, message, false );
    
    typedef my_visualizer< embedded_watermark_t::element_type > visualizer_t; 
    visualizer_t v( ew.get() ); 
    visualization::viewer< visualizer_t > viewer( &v, "Watermarking", &argc, argv );
}
