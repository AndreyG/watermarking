#include "qtviewer.h"

struct subdivided_plane_viewer_t : viewer_t
{
    typedef geometry::planar_graph_t graph_t;

    subdivided_plane_viewer_t(  graph_t const * graph, 
                                std::vector<size_t> const * subdivision,
                                size_t areas_num )
        : graph_(graph)
        , subdivision_(subdivision)
        , color_(areas_num)
    {
        srand(239);
        for (size_t i = 0; i != areas_num; ++i)
            color_[i] = QColor(rand() % 256, rand() % 256, rand() % 256);
    }

    void draw(drawer_t & drawer) const
    {
        for (size_t v = 0; v != graph_->vertices_num(); ++v)
        {
            drawer.set_color(color_[subdivision_->at(v)]);
            drawer.draw_point(graph_->vertex(v), 2);
        }
        for (auto edge = graph_->edges_begin(); edge != graph_->edges_end(); ++edge)
        {
            size_t beg = edge->b;
            size_t end = edge->e;

            graph_t::vertex_t const & v_beg = graph_->vertex(beg);
            graph_t::vertex_t const & v_end = graph_->vertex(end);

            if ((*subdivision_)[beg] == (*subdivision_)[end])
            {
                drawer.set_color(color_[(*subdivision_)[beg]]);
                drawer.draw_line(v_beg, v_end);
            }
            else
            {
                drawer.set_color(color_[subdivision_->at(beg)]);
                drawer.draw_line(v_beg, v_end);
                drawer.set_color(color_[subdivision_->at(end)]);
                drawer.draw_line(v_beg, v_end);
            }
        }
    }

private:
    graph_t const * graph_;
    std::vector<size_t> const * subdivision_;
    std::vector<QColor> color_;
};
