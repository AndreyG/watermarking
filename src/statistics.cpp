#include "stdafx.h"

#include "statistics.h"

#include "algorithms/util.h"

double angle(geometry::point_t const & u, geometry::point_t const & v, geometry::point_t const & w)
{
    assert(u != v);
    assert(w != v);
    assert(u != w);

	auto v1 = u - v;
	auto v2 = w - v;

    double c = (v1 * v2) / (sqrt(mod(v1)) * sqrt(mod(v2)));
    using algorithm::make_min;
    using algorithm::make_max;
	assert(!isnan(c));
    make_min(c, 1.0);
    make_max(c, -1.0);
    return acos(c);
}

typedef geometry::planar_graph_t graph_t;

bool loops_exist(graph_t const &);
bool has_duplicate_vertices(graph_t const &);

double abs(double x)
{
    return (x > 0) ? x : -x;
}

tuple<double, size_t> angle_difference(graph_t const& g1, graph_t const& g2)
{
    assert(!loops_exist(g1));
    assert(!loops_exist(g2));
    assert(!has_duplicate_vertices(g1));
    assert(!has_duplicate_vertices(g2));

    const size_t N = g1.vertices_num();
    std::vector< std::set< size_t > > edges( N );
    for (auto e = g1.edges_begin(); e != g1.edges_end(); ++e)
    {
        edges[e->b].insert(e->e);
        edges[e->e].insert(e->b);
    }
    double res = 0.0;
    size_t angles_num = 0;
    for (size_t v = 0; v != N; ++v)
    {
        if (edges[v].size() >= 2)
        {
			graph_t::vertex_t const & p1 = g1.vertex(v);
			graph_t::vertex_t const & p2 = g2.vertex(v);
            std::vector< size_t > e( edges[v].begin(), edges[v].end() );
            for (size_t i = 0; i + 1 != e.size(); ++i)
            {
                for (size_t j = i + 1; j != e.size(); ++j)
                {
                    ++angles_num;
                    assert(e[i] != v);
                    assert(e[j] != v);
                    double a1 = angle(g1.vertex(e[i]), p1, g1.vertex(e[j]));
                    double a2 = angle(g2.vertex(e[i]), p2, g2.vertex(e[j]));
                    assert(a1 >= 0);
                    assert(a1 < 3.15);
                    assert(a2 >= 0);
                    assert(a2 < 3.15);
                    res += abs(a1 - a2);
                }
            }
        }
    }
    return make_tuple(res, angles_num);
}

