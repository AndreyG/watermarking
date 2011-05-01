#include "qtviewer.h"

struct subdivided_plane_viewer_t : viewer_t
{
    void draw(drawer_t & drawer) const
    {
        using geometry::point_t;

        drawer.set_color(Qt::red);
        drawer.draw_line(point_t(-1.0, -1.0), point_t(1.0, 1.0));

        //renderText(5.0, 7.0, 0.0, "xui!");
    }
};
