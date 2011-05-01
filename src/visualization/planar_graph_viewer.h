#include "qtviewer.h"
#include "../geometry/planar_graph.h"

struct planar_graph_viewer_t : viewer_t
{
    typedef geometry::planar_graph_t graph_t;
    
    explicit planar_graph_viewer_t(graph_t const * graph)
        : graph_(graph)
    {}

    void draw(drawer_t & drawer) const
    {
        drawer.set_color(Qt::blue);
        for (graph_t::vertices_iterator v = graph_->vertices_begin(); v != graph_->vertices_end(); ++v) 
            drawer.draw_point(*v, 2);

        drawer.set_color(Qt::green);
        for (graph_t::edges_iterator it = graph_->edges_begin(); it != graph_->edges_end(); ++it)
            drawer.draw_line(graph_->vertex(it->b), graph_->vertex(it->e));

        *drawer.corner_stream() << "vertices num: " << graph_->vertices_num();
        *drawer.corner_stream() << "edges num: " << graph_->edges_num();
    }

private:
    graph_t const * graph_;
};
