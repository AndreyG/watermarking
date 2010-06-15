#include <fstream>
#include <vector>
#include <cassert>
#include <sstream>
#include <iostream>

#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

int main( int argc, char** argv )
{
    assert( argc == 3 );
    std::ifstream in (argv[1]);
    std::ofstream out(argv[2]);

    typedef std::pair< double, double > vertex_t;
    std::vector< vertex_t > vertices;
    
    typedef std::pair< size_t, size_t > edge_t;
    std::vector< edge_t > edges;

    while ( in )
    {
        std::string s = "";
        while ( in && s.empty() )
            in >> s;
        if ( !in )
            break;
        std::stringstream ss( s, std::stringstream::in );
        size_t start_idx = vertices.size();
        std::cout << start_idx << std::endl;
        while ( ss )
        {
            double x, y;
            char c;
            do
            {
                ss >> c;
            } while ( c != '(' && ss );
            if ( !ss )
                break;
            ss >> x;
            do
            {
                ss >> c;
            } while ( c != ',' );
            ss >> y;
            do
            {
                ss >> c;
            } while ( c != ')' );
            vertices.push_back( vertex_t( x, y ) );
        }
        for ( size_t i = start_idx; i + 1 != vertices.size(); ++i )
            edges.push_back( edge_t( i, i + 1 ) );
        edges.push_back( edge_t( start_idx, vertices.size() - 1 ) );
    }
    out << vertices.size() << "\t" << edges.size() << std::endl;
    foreach( vertex_t const & v, vertices )
        out << "(" << v.first << ", " << v.second << ")" << "\n";
    out << std::endl;
    foreach( edge_t const & e, edges )
        out << e.first << "\t" << e.second << "\n";
}
