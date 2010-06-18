#include <vector>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <boost/utility.hpp>
#include <assert.h>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/scoped_array.hpp>
#include <boost/optional.hpp>

#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

#include <boost/program_options.hpp>
#include <boost/format.hpp>

#include "utility/dump.h"
#include "utility/stopwatch.h"

#include "watermarking/embedding.h"
#include "visualization/my_visualizer.h"

template< class Stream, class Point >
void read_points( Stream & in, const size_t N, Point * out )
{
    typedef Point point_t;

    for ( size_t i = 0; i != N; ++i )
    {
        double x, y;
        util::skip_char( in, '(' );
        in >> x;
        util::skip_char( in, ',' );
        in >> y;
        util::skip_char( in, ')' );   
        *out++ = point_t( x, y );
    }
}

template< class Stream, class Edge >
void read_edges( Stream & in, const size_t N, Edge * out )
{
    typedef Edge edge_t;
    for ( size_t i = 0; i != N; ++i )
    {
        size_t v, u;
        in >> v >> u;
        *out++ = edge_t( v, u );
    }
}

template< class Graph, class Stream >
void read_graph( Graph & graph, Stream & in )
{
    size_t n, m;
    in >> n >> m;
    graph.vertices.resize( n );
    graph.edges.resize( m );

    read_points( in, n, &graph.vertices[0] );
    typedef typename Graph::vertex_t vertex_t; 
    double middle_x = 0, middle_y = 0;
    foreach ( vertex_t const & v, graph.vertices )
    {
        middle_x += v.x();
        middle_y += v.y();
    }
    middle_x /= graph.vertices.size();
    middle_y /= graph.vertices.size();
    foreach ( vertex_t & v, graph.vertices )
    {
        v = vertex_t( v.x() - middle_x, v.y() - middle_y );
    }

    read_edges( in, m, &graph.edges[0] );
}

template< class Graph >
void fix_graph( Graph & g )
{
    typedef typename Graph::vertex_t vertex_t;
    typedef std::map< vertex_t, size_t > v2i_t;  
    v2i_t v2i;
    size_t idx = 0;
    foreach ( vertex_t const & v, g.vertices )
    {
        typename v2i_t::iterator it = v2i.find( v );
        if ( it == v2i.end() )
        {    
            v2i.insert( it, std::make_pair( v, idx ) );
            ++idx;
        }
    }
    typename Graph::vertices_t new_vertices;
    std::set< vertex_t > vertex_set;
    foreach ( vertex_t const & v, g.vertices )
    {
        if ( !vertex_set.count( v ) )
        {
            vertex_set.insert( v );
            new_vertices.push_back( v );
        }
    }

    typename Graph::edges_t new_edges;
    typedef typename Graph::edge_t edge_t;
    foreach ( edge_t const & e, g.edges )
    {
        new_edges.push_back( edge_t( v2i[g.vertices[e.first]], v2i[g.vertices[e.second]] ) );
    }

    g.vertices.swap( new_vertices );
    g.edges.swap( new_edges );
}

boost::program_options::variables_map read_params()
{
    namespace po = boost::program_options;

    std::ifstream conf("common.conf");
    po::options_description desc;
    desc.add_options()
        ( "step-by-step", po::value<bool>() )
        ( "weighted",     po::value<bool>() )
        ( "use-edges",    po::value<bool>() )
        ( "input-data",   po::value<std::string>() )
    ;
    
    po::variables_map vm;       
    po::store(po::parse_config_file(conf, desc), vm);

    return vm;
}

int main( int argc, char** argv )
{
    boost::program_options::variables_map params = read_params();

    std::ifstream in( params["input-data"].as< std::string >().c_str() );
    
    typedef CGAL::Exact_predicates_inexact_constructions_kernel::Point_2 point_t; 
    typedef watermarking::planar_graph< point_t > graph_t;

    graph_t graph;
    read_graph( graph, in );
    fix_graph( graph );
    
    typedef std::auto_ptr< watermarking::embedding_impl< graph_t::vertex_t > > embedded_watermark_t;
    bool weighted = params["weighted"].as< bool >();
    bool use_edges = params["use-edges"].as< bool >();
    embedded_watermark_t ew = watermarking::embed( graph, weighted, use_edges, params["step-by-step"].as< bool >() );
                                                             
    typedef my_visualizer< embedded_watermark_t::element_type > visualizer_t; 
    visualizer_t v( ew.get() ); 
    boost::format title("W: weighted = %1%, use-edges = %2%");
    visualization::viewer< visualizer_t > viewer( &v, ( title % weighted % use_edges ).str().c_str(), &argc, argv );
}
