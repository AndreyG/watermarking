#include "qtviewer.h"
#include "../geometry/planar_graph.h"

struct graph_diff_viewer_t : visualization::viewer_t
{
    typedef geometry::planar_graph_t graph_t;

    graph_diff_viewer_t(graph_t const * original_graph,
                        graph_t const * modified_graph,
                        graph_t const * noised_graph,
                        const char * first_graph_name,
                        const char * second_graph_name, 
                        const char * third_graph_name )
    {
        graph_[0] = original_graph;
        graph_[1] = modified_graph;
        graph_[2] = noised_graph;

        vertex_color_[0] = edge_color_[0] = Qt::green;
        vertex_color_[1] = edge_color_[1] = Qt::blue;
        vertex_color_[2] = edge_color_[2] = Qt::red;

        std::fill(draw_.begin(), draw_.end(), true);

        graph_name_[0] = first_graph_name;
        graph_name_[1] = second_graph_name;
        graph_name_[2] = third_graph_name;
    }

    void draw(visualization::drawer_t & drawer) const
    {
        for (int i = 0; i != 3; ++i) if (draw_[i])
            draw_graph(drawer, graph_[i], vertex_color_[i], edge_color_[i]);
    }

    void print(visualization::printer_t & prn) const
    {
        for (int i = 0; i != 3; ++i)
            *prn.corner_stream() << "key " << (i + 1) << ": " << graph_name_[i];
    }

    bool on_key(int key)
    {
        key -= Qt::Key_1;
        if ((key >= 0) && (key <= 2))
        {
            draw_[key] = !draw_[key];
            return true;
        }
        return false;
    };

private:
    void draw_graph(visualization::drawer_t & drawer, graph_t const * graph, 
                    QColor const & vertex_color, QColor const & edge_color) const
    {
        drawer.set_color(vertex_color);
        for (graph_t::vertices_iterator v = graph->vertices_begin(); v != graph->vertices_end(); ++v) 
            drawer.draw_point(*v, 2);

        drawer.set_color(edge_color);
        for (graph_t::edges_iterator it = graph->edges_begin(); it != graph->edges_end(); ++it)
            drawer.draw_line(graph->vertex(it->b), graph->vertex(it->e));
    }

private:
    boost::array<graph_t const *, 3> graph_;
    boost::array<bool, 3> draw_;
    boost::array<std::string, 3> graph_name_;
    boost::array<QColor, 3> vertex_color_;
    boost::array<QColor, 3> edge_color_;
};
