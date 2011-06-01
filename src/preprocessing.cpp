#include "stdafx.h"
#include "geometry/planar_graph.h"
#include "inout/inout.h"

typedef geometry::planar_graph_t graph_t;

bool loops_exist(graph_t const &);
bool has_duplicate_vertices(graph_t const &);
void fix_graph(graph_t & graph);

int main(int argc, char ** argv)
{
    assert(argc == 2);

    graph_t g;
    {
        std::ifstream in(argv[1]);
        inout::read_graph(g, in);
    }

    fix_graph(g);
    assert(!has_duplicate_vertices(g));
    assert(!loops_exist(g));

    {
        std::ofstream out(argv[1]);
        inout::write_graph(g, out);
    }
}
