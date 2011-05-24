#include "../stdafx.h"

#include "drawer_impl.h"

namespace visualization
{

void drawer_impl::clear()
{
    point_buffers.clear();
    segment_buffers.clear();
}

void drawer_impl::set_color(QColor const & c)
{
    current_color_ = c;
}

void drawer_impl::draw_line(point_t const & a, point_t const & b, double width)
{
    if (segment_buffers.empty() || (segment_buffers.back().width != width))
    {
        segment_buffers.push_back(segment_buffer_t());
        segment_buffers.back().width = width;
    }
    
    std::vector<GLdouble> & points_buffer = segment_buffers.back().segments;
    points_buffer.push_back(a.x());
    points_buffer.push_back(a.y());
    points_buffer.push_back(b.x());
    points_buffer.push_back(b.y());

    std::vector<GLdouble> & colors_buffer = segment_buffers.back().colors;
    colors_buffer.push_back(current_color_.redF());
    colors_buffer.push_back(current_color_.greenF());
    colors_buffer.push_back(current_color_.blueF());
    colors_buffer.push_back(current_color_.redF());
    colors_buffer.push_back(current_color_.greenF());
    colors_buffer.push_back(current_color_.blueF());
}

void drawer_impl::draw_point(point_t const & pt, size_t radius)
{
    if (point_buffers.empty() || (point_buffers.back().radius != radius))
    {
        point_buffers.push_back(point_buffer_t());
        point_buffers.back().radius = radius;
    }
    
    std::vector<GLdouble> & points_buffer = point_buffers.back().points;
    points_buffer.push_back(pt.x());
    points_buffer.push_back(pt.y());

    std::vector<GLdouble> & colors_buffer = point_buffers.back().colors;
    colors_buffer.push_back(current_color_.redF());
    colors_buffer.push_back(current_color_.greenF());
    colors_buffer.push_back(current_color_.blueF());
}

}
