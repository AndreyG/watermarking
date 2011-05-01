#include "qtviewer.h"

template< class Triangulation >
struct triangulation_graph_viewer_t : viewer_t
{
    typedef geometry::triangulation_graph<Triangulation> graph_t;

    explicit triangulation_graph_viewer_t(graph_t const * graph)
        : graph_(graph)
    {}

    void draw(drawer_t & drawer) const
    {
        drawer.set_color(Qt::blue);
        for (size_t i = 0; i != graph_->vertices_num(); ++i)
            drawer.draw_point(graph_->vertex(i), 2);

        drawer.set_color(Qt::green);
        for (size_t i = 0; i != graph_->edges_num(); ++i)
        {
            typename graph_t::edge_t const & edge = graph_->edge(i);
            drawer.draw_line(graph_->vertex(edge.b), graph_->vertex(edge.e));
        }
    }

private:
    graph_t const * graph_;
};
