#include "qtviewer.h"
#include "../geometry/planar_graph.h"

struct planar_graph_viewer_t : visualization::viewer_t
{
    typedef geometry::planar_graph_t graph_t;
    
    explicit planar_graph_viewer_t(graph_t const * graph)
        : graph_(graph)
        , draw_vertices_(true)
        , draw_edges_(true)
    {}

    void draw(visualization::drawer_t & drawer) const
    {
        if (draw_vertices_)
        {
            drawer.set_color(Qt::blue);
            for (graph_t::vertices_iterator v = graph_->vertices_begin(); v != graph_->vertices_end(); ++v) 
                drawer.draw_point(*v, 5);
        }

        if (draw_edges_)
        {
            drawer.set_color(Qt::green);
            for (graph_t::edges_iterator it = graph_->edges_begin(); it != graph_->edges_end(); ++it)
                drawer.draw_line(graph_->vertex(it->b), graph_->vertex(it->e));
        }
    }

    bool on_key(int key) 
    {
        switch (key)
        {
        case 'V':
            draw_vertices_ = !draw_vertices_;
            return true;
        case 'E':
            draw_edges_ = !draw_edges_;
            return true;
        default:
            return false;
        }
    }

    void print(visualization::printer_t & prn) const
    {
        *prn.corner_stream() << "vertices num: "    << graph_->vertices_num();
        *prn.corner_stream() << "edges num: "       << graph_->edges_num();
    }    

private:
    graph_t const * graph_;
    bool draw_vertices_, draw_edges_;
};
