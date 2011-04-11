#include "stdafx.h"
#include "geometry/planar_graph.h"
#include "inout/inout.h"

typedef geometry::planar_graph_t graph_t;

void fix_graph( graph_t & graph );

int main(int argc, char ** argv)
{
    assert(argc == 2);

    graph_t g;
    {
        std::ifstream in(argv[1]);
        inout::read_graph(g, in);
    }

    fix_graph(g);

    {
        std::ofstream out(argv[1]);
        inout::write_graph(g, out);
    }
}
