#ifndef _QT_VIEWER_H_
#define _QT_VIEWER_H_

#include "../geometry/point.h"

struct stream_t
{
#define PRINT(type) virtual stream_t & operator << (type) = 0;
    PRINT(const char *)
    PRINT(geometry::point_t const &)
#undef PRINT
    virtual ~stream_t();
};

struct drawer_t
{
    virtual void set_color(QColor const & c) = 0;
    virtual void draw_line( geometry::point_t const & a,
                            geometry::point_t const & b) = 0;
    virtual std::unique_ptr<stream_t> corner_stream() = 0;
    virtual ~drawer_t();
};

struct viewer_t
{
    virtual void draw(drawer_t &) const = 0;
    virtual ~viewer_t();
};

struct vis_system
{
public:
    static void init(int argc, char** argv);
    static void run_viewer(viewer_t const * viewer);
private:
    static boost::scoped_ptr<QApplication> app;
    static boost::scoped_ptr<QGLWidget>    fake_widget;
};

#endif
