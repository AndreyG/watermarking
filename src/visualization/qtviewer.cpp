#include "../stdafx.h"
#include "qtviewer.h"

#include "../utility/stopwatch.h"
#include "../utility/debug_stream.h"
#include "../geometry/point.h"

stream_t::~stream_t() {}
drawer_t::~drawer_t() {}
viewer_t::~viewer_t() {}

boost::scoped_ptr<QApplication> vis_system::app;
boost::scoped_ptr<QGLWidget>    vis_system::fake_widget;

void vis_system::init(int argc, char ** argv)
{
    app.reset(new QApplication(argc, argv));
    fake_widget.reset(new QGLWidget);
}

namespace
{
using geometry::point_t;

struct main_window_t : QGLWidget, drawer_t
{
    explicit main_window_t(viewer_t const * viewer);

    void set_color(QColor const & c);
    void draw_line(point_t const & a, point_t const & b);
    std::unique_ptr<stream_t> corner_stream();

private:
    void initializeGL();
    void resizeGL(int, int);
    void paintGL();

    void wheelEvent(QWheelEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);

    void resize_impl(int, int);
    point_t screen_to_global(QPoint const & screen_pos) const;

private:
    void draw_string(int x, int y, const char * s);


private:
    viewer_t const * viewer_;

    point_t center_;
    point_t size_;
    point_t current_pos_;
    double  zoom_;
    int corner_stream_height_indent_;
    boost::optional<point_t> start_point_;
};

main_window_t::main_window_t(viewer_t const * viewer)
    : viewer_(viewer)
    , size_(100, 100)
    , current_pos_(center_)
    , zoom_(1.0)
{
    setMouseTracking(true);
}

void main_window_t::set_color(QColor const & c)
{
    qglColor(c);
}

void main_window_t::draw_line(point_t const & a, point_t const & b)
{
    glBegin(GL_LINE);
        glVertex2d(a.x(), a.y());
        glVertex2d(b.x(), b.y());
    glEnd();
}

void main_window_t::initializeGL()
{
	assert(doubleBuffer());
    setAutoBufferSwap(true);
    qglClearColor(Qt::black);
}

void main_window_t::resize_impl(int screen_w, int screen_h)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    point_t size(screen_w, screen_h);
    size *= zoom_;

    point_t left_bottom = center_ - (size / 2);
    point_t right_top   = center_ + (size / 2);

    glOrtho(left_bottom.x(), right_top.x(), left_bottom.y(), right_top.y(), -1.0, 1.0);
    glViewport(0, 0, screen_w, screen_h);
}

void main_window_t::resizeGL(int screen_w, int screen_h)
{
    resize_impl(screen_w, screen_h);
}

void main_window_t::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);

    corner_stream_height_indent_ = 15;
    *corner_stream() << "Mouse pos: " << current_pos_;
    viewer_->draw(*this);
}

void main_window_t::wheelEvent(QWheelEvent * e)
{
    if (e->delta() > 0)
        zoom_ *= 1.1;
    else if (e->delta() < 0)
        zoom_ /= 1.1;
    e->accept();

    resize_impl(size().width(), size().height());
    updateGL();
}

void main_window_t::mousePressEvent(QMouseEvent * event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        start_point_ = screen_to_global(event->pos());
    }
}

void main_window_t::mouseMoveEvent(QMouseEvent * event)
{
    current_pos_ = screen_to_global(event->pos());

    if (start_point_)
    {
        center_ -= (current_pos_ - *start_point_);
        start_point_ = current_pos_;
        resize_impl(size().width(), size().height());
    }

    updateGL();
}

void main_window_t::mouseReleaseEvent(QMouseEvent * )
{
    start_point_ = boost::none;
}

point_t main_window_t::screen_to_global(QPoint const & screen_pos) const
{
    const int w = size().width();
    const int h = size().height();
    point_t screen_size(w, h);
    point_t pos(screen_pos.x(), h - screen_pos.y());
    return center_ - (screen_size / 2 - pos) * zoom_;
}

struct stream_impl : stream_t
{
    explicit stream_impl(boost::function<void (const char *)> const & write)    
        : write_(write)
    {}

#define PRINT(type, t) \
    stream_t & operator << (type t) \
    { \
        ss_ << t; \
        return *this; \
    }
    PRINT(const char *, s)
    PRINT(point_t const &, pt)
#undef PRINT

    ~stream_impl() { write_(ss_.str().c_str()); }

private:
    std::stringstream ss_;
    boost::function<void (const char*)> write_; 
};

void main_window_t::draw_string(int x, int y, const char * s)
{
    set_color(Qt::white);
    renderText(x, y, s);
}

std::unique_ptr<stream_t> main_window_t::corner_stream()
{
    std::unique_ptr<stream_t> stream(new stream_impl(boost::bind(&main_window_t::draw_string, this, 10, corner_stream_height_indent_, _1)));
    corner_stream_height_indent_ += 15;
    return stream;
}

}

void vis_system::run_viewer(viewer_t const * viewer)
{
    main_window_t wnd(viewer);
    wnd.show();
    app->exec(); 
}
