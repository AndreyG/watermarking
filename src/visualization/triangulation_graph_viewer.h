#ifndef _TRIANGULATION_GRAPH_VIEWER_H_
#define _TRIANGULATION_GRAPH_VIEWER_H_

#include "qtviewer.h"

template< class Triangulation, class Scalar >
struct triangulation_graph_viewer_t : viewer_t
{
    typedef geometry::triangulation_graph<Triangulation> graph_t;

    triangulation_graph_viewer_t(graph_t const * graph, std::vector<Scalar> const & weight)
        : graph_(graph)
        , weight_(weight)
    {}

    void draw(drawer_t & drawer) const
    {
        const size_t N = graph_->vertices_num();

        drawer.set_color(Qt::blue);
        for (size_t i = 0; i != N; ++i)
            drawer.draw_point(graph_->vertex(i), 2);

        using geometry::point_t;

        drawer.set_color(Qt::green);
        for (size_t i = 0; i != graph_->edges_num(); ++i)
        {
            typename graph_t::edge_t const & edge = graph_->edge(i);
            
            point_t const & beg = graph_->vertex(edge.b);
            point_t const & end = graph_->vertex(edge.e); 
            drawer.draw_line(beg, end);

            *drawer.global_stream((beg + end) / 2) << weight_[N * edge.e + edge.b];
        }
    }

private:
    graph_t const * graph_;
    std::vector<Scalar> const & weight_;
};

#endif
