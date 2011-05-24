#ifndef _DRAWER_IMPL_H_ 
#define _DRAWER_IMPL_H_ 

#include "../geometry/point.h"
#include "qtviewer.h"

namespace visualization
{

using geometry::point_t;

struct drawer_impl : drawer_t
{
    void set_color(QColor const & c);
    void draw_line(point_t const & a, point_t const & b, double width);
    void draw_point(point_t const & pt, size_t radius);

    drawer_impl()
        : current_color_ (Qt::black)
    {}

    void clear();

    struct point_buffer_t
    {
        std::vector<GLdouble> points;
        std::vector<GLdouble> colors;
        size_t radius;
    };

    struct segment_buffer_t
    {
        std::vector<GLdouble> segments;
        std::vector<GLdouble> colors;
        double width;
    };

    std::vector<point_buffer_t>     point_buffers;
    std::vector<segment_buffer_t>   segment_buffers;

private:
    QColor current_color_;
};

}

#endif /*_DRAWER_IMPL_H_*/
