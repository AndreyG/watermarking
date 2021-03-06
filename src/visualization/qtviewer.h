#ifndef _QT_VIEWER_H_
#define _QT_VIEWER_H_

#include "../geometry/point.h"

namespace visualization 
{

struct stream_t
{
#define PRINT(type) virtual stream_t & operator << (type) = 0;
    PRINT(const char *)
    PRINT(std::string const &)
    PRINT(size_t)
    PRINT(std::complex<double> const &)
    PRINT(geometry::point_t const &)
#undef PRINT
    virtual ~stream_t();
};

struct drawer_t
{
    virtual void set_color(QColor const & c) = 0;
    virtual void draw_line( geometry::point_t const & a,
                            geometry::point_t const & b,
                            double width = 1.0 ) = 0;
    virtual void draw_point(geometry::point_t const & pt, size_t radius = 1) = 0;
    virtual ~drawer_t();
};

struct printer_t
{
    virtual std::unique_ptr<stream_t> corner_stream() = 0;
    virtual std::unique_ptr<stream_t> global_stream(geometry::point_t const & pt) = 0;

    virtual ~printer_t();
};

struct viewer_t
{
    virtual void draw(drawer_t &)   const = 0;
    virtual void print(printer_t &) const {};
    virtual bool on_key(int /*key*/) { return false; }
    virtual ~viewer_t();
};

void run_viewer(viewer_t * viewer);

} // namespace visualization

#endif
