#include "stdafx.h"

#include "geometry/planar_graph.h"
#include "inout/inout.h"

using geometry::point_t;
typedef geometry::planar_graph_t graph_t;

template<class Contour>
void add_contour(graph_t & graph, Contour const & cnt)
{
    const size_t V = graph.vertices_num();

    graph.add_vertices(cnt.begin(), cnt.end());

    for (size_t i = 0, N = cnt.size(); i != N; ++i)
    {
        graph_t::edge_t edge;
        edge.b = V + i;
        edge.e = V + (i + 1) % N;

        graph.add_edge(edge);
    }
}

template<class ContoursIterator>
graph_t build_graph(ContoursIterator begin, ContoursIterator end)
{
    graph_t res;

    for ( ; begin != end; ++begin )
        add_contour(res, *begin);

    return res;
}

template<class OutIter>
void read_contours(const char * filename, OutIter out)
{
    typedef std::istream_iterator<point_t> iter_t;

    std::ifstream in(filename);
    for ( std::string s; in >> s; )
    {
        std::stringstream ss(s);
        std::vector<point_t> vec;
        vec.assign(iter_t(ss), iter_t());
        *out++ = vec;
    }
}

int main(int argc, char ** argv)
{
    assert(argc == 2);
    const char * filename = argv[1];

    std::vector<std::vector<geometry::point_t>> contours;
    read_contours(filename, std::back_inserter(contours));

    auto graph = build_graph(contours.begin(), contours.end());
    std::ofstream out(filename);
    inout::write_graph(graph, out);
}
