#include "utility/dump.h"

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

    read_edges( in, m, &graph.edges[0] );
}

template< class Stream, class Point >
Stream& dump( Stream & out, Point const & pt )
{
     out << "(" << pt.x() << ", " << pt.y() << ")";
     return out;
}

template< class Graph, class Stream >
void dump_graph( Graph const & graph, Stream & out )
{
    out << graph.vertices.size() << "\t" << graph.edges.size() << "\n";
    foreach (typename Graph::vertex_t const & v, graph.vertices)
    {
        dump(out, v) << "\n";
    }
    foreach (typename Graph::edge_t const & e, graph.edges)
    {
        out << e.first << " " << e.second << "\n";
    }
}

struct message_params_t
{
    std::string text;
    size_t      chip_rate;
    int         key;
    double      alpha;
};

message_params_t read_message_params(const char * filepath)
{
    std::ifstream conf(filepath);
                
    namespace po = boost::program_options;

    po::options_description desc;
    desc.add_options()
        ( "message", po::value< std::string >() )
        ( "key",     po::value< int >() )
        ( "chip-rate", po::value< size_t >() )
        ( "alpha",   po::value< double >() )
    ;

    po::variables_map vm;       
    po::store( po::parse_config_file( conf, desc ), vm );

    message_params_t params;
    params.text = vm["message"].as< std::string >();
    params.key  = vm["key"].as< int >();
    params.chip_rate = vm["chip-rate"].as< size_t >();
    params.alpha = vm["alpha"].as< double >();

    return params;
}

template< class Graph >
void fix_graph( Graph & g )
{
    typedef typename Graph::vertex_t vertex_t;
    typedef typename Graph::edge_t edge_t;
    typedef std::map< vertex_t, size_t > v2i_t;  

    typedef typename Graph::vertex_t vertex_t; 
    
    double middle_x = 0, middle_y = 0;
    foreach ( vertex_t const & v, g.vertices )
    {
        middle_x += v.x();
        middle_y += v.y();
    }
    middle_x /= vertices_num( g );
    middle_y /= vertices_num( g );
    foreach ( vertex_t & v, g.vertices )
    {
        v = vertex_t( v.x() - middle_x, v.y() - middle_y );
    }

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
    foreach ( edge_t const & e, g.edges )
    {
        size_t v = v2i[g.vertices[e.first]];
        size_t u = v2i[g.vertices[e.second]]; 
        if (v != u)
            new_edges.push_back( edge_t( v, u ) );
    }

    g.vertices.swap( new_vertices );
    g.edges.swap( new_edges );
}
