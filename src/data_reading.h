#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Constrained_Delaunay_triangulation_2.h>

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
void add_intersection_points( Graph & graph )
{
    util::stopwatch _("fixing intersections in input graph");

    typedef CGAL::Exact_predicates_inexact_constructions_kernel         GT;
    typedef CGAL::Exact_predicates_tag                                  Itag;
    typedef CGAL::Triangulation_vertex_base_2<GT>                       Vb;
    typedef CGAL::Constrained_triangulation_face_base_2<GT>             Fb;
    typedef CGAL::Triangulation_data_structure_2<Vb, Fb>                TDS;
    typedef CGAL::Constrained_Delaunay_triangulation_2<GT, TDS, Itag>   CDT;

    CDT trg;
    
    typedef typename Graph::vertex_t vertex_t;
    typedef typename Graph::edge_t edge_t;
    
    foreach ( edge_t const & e, graph.edges )
    {
        trg.insert_constraint(graph.vertices[e.first], graph.vertices[e.second]);
    }

    graph.vertices.clear();
    graph.edges.clear();

    std::map< CDT::Vertex_handle, size_t > handle_to_index;
    for ( auto v = trg.finite_vertices_begin(); v != trg.finite_vertices_end(); ++v )
    {
        handle_to_index[v] = graph.vertices.size();
        graph.vertices.push_back(v->point());
    }

    for ( auto e = trg.finite_edges_begin(); e != trg.finite_edges_end(); ++e )
    {
        auto face = e->first;
        int v = e->second;
        size_t a = handle_to_index[face->vertex(trg.cw(v))];
        size_t b = handle_to_index[face->vertex(trg.ccw(v))];
        graph.edges.push_back( edge_t( a, b ) );
    }
}

template< class Graph >
void fix_graph( Graph & g )
{
    add_intersection_points( g );
    typedef typename Graph::vertex_t vertex_t;
    typedef typename Graph::edge_t edge_t;
    typedef std::map< vertex_t, size_t > v2i_t;  

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
