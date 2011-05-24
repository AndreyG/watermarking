#ifndef _PRINTER_IMPL_H_
#define _PRINTER_IMPL_H_

#include "../geometry/point.h"
#include "qtviewer.h"

namespace visualization
{

using geometry::point_t;

struct printer_impl : printer_t
{
    std::unique_ptr<stream_t> corner_stream();
    std::unique_ptr<stream_t> global_stream(point_t const & pt);

    printer_impl(   boost::function<void (int, int, const char *)>          const & draw_string_corner,
                    boost::function<void (double, double, const char *)>    const & draw_string_global)
        : corner_stream_height_indent_(15)
        , draw_string_corner_(draw_string_corner)
        , draw_string_global_(draw_string_global)
    {}

private:
    int corner_stream_height_indent_;
    boost::function<void (int, int, const char *)>          draw_string_corner_;
    boost::function<void (double, double, const char *)>    draw_string_global_;
};

}

#endif /*_PRINTER_IMPL_H_*/
