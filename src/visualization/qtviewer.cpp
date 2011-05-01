#include "../stdafx.h"
#include "qtviewer.h"

#include "../utility/stopwatch.h"
#include "../utility/debug_stream.h"
#include "../geometry/point.h"
#include "../inout/inout.h"

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

struct drawer_impl : drawer_t
{
    void set_color(QColor const & c);
    void draw_line(point_t const & a, point_t const & b);
    void draw_point(point_t const & pt, size_t radius);
    std::unique_ptr<stream_t> corner_stream();

    drawer_impl(boost::function<void (int, int, const char *)> const & draw_string)
        : current_color_ (Qt::black)
        , corner_stream_height_indent_(15)
        , draw_string_(draw_string)
    {}

    struct point_buffer_t
    {
        std::vector<GLdouble> points;
        QColor color;
        size_t radius;
    };

    struct segment_buffer_t
    {
        std::vector<GLdouble> segments;
        QColor color;
    };

    std::vector<point_buffer_t>     point_buffers;
    std::vector<segment_buffer_t>   segment_buffers;

private:
    QColor current_color_;
    int corner_stream_height_indent_;
    boost::function<void (int, int, const char *)> draw_string_;
};

void drawer_impl::set_color(QColor const & c)
{
    current_color_ = c;
}

void drawer_impl::draw_line(point_t const & a, point_t const & b)
{
    if (segment_buffers.empty() || (segment_buffers.back().color != current_color_))
    {
        segment_buffers.push_back(segment_buffer_t());
        segment_buffers.back().color = current_color_;
    }
    std::vector<GLdouble> & buffer = segment_buffers.back().segments;
    buffer.push_back(a.x());
    buffer.push_back(a.y());
    buffer.push_back(b.x());
    buffer.push_back(b.y());
}

void drawer_impl::draw_point(point_t const & pt, size_t radius)
{
    if (point_buffers.empty() || (point_buffers.back().color != current_color_) ||
        (point_buffers.back().radius != radius))
    {
        point_buffers.push_back(point_buffer_t());
        point_buffers.back().color = current_color_;
        point_buffers.back().radius = radius;
    }
    std::vector<GLdouble> & buffer = point_buffers.back().points;
    buffer.push_back(pt.x());
    buffer.push_back(pt.y());
}

struct stream_impl : stream_t
{
    explicit stream_impl(boost::function<void (const char *)> const & write)    
        : write_(write)
    {}

#define PRINT(type) \
    stream_t & operator << (type t) \
    { \
        ss_ << t; \
        return *this; \
    }
    PRINT(const char *)
    PRINT(size_t)
    PRINT(point_t const &)
#undef PRINT

    ~stream_impl() { write_(ss_.str().c_str()); }

private:
    std::stringstream ss_;
    boost::function<void (const char*)> write_; 
};

std::unique_ptr<stream_t> drawer_impl::corner_stream()
{
    std::unique_ptr<stream_t> stream(new stream_impl(boost::bind(draw_string_, 10, corner_stream_height_indent_, _1)));
    corner_stream_height_indent_ += 15;
    return stream;
}

struct main_window_t : QGLWidget
{
public:
    explicit main_window_t(viewer_t const * viewer);
    ~main_window_t() {}

private:
    void initializeGL();
    void resizeGL(int, int);
    void paintGL();

    void wheelEvent(QWheelEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void keyReleaseEvent(QKeyEvent *);

    void resize_impl(int, int);
    point_t screen_to_global(QPoint const & screen_pos) const;

private slots:
    void repaint();

private:
    void draw_string(int x, int y, const char * s);

private:
    viewer_t const * viewer_;


    point_t center_;
    point_t size_;
    point_t current_pos_;
    double  zoom_;
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

void main_window_t::initializeGL()
{
	assert(doubleBuffer());
    setAutoBufferSwap(true);
    qglClearColor(Qt::black);
}

void main_window_t::repaint()
{
    updateGL();
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
    util::stopwatch _("paintGL");

    glClear(GL_COLOR_BUFFER_BIT);

    drawer_impl drawer(boost::bind(&main_window_t::draw_string, this, _1, _2, _3));

    *drawer.corner_stream() << "Mouse pos: " << current_pos_;
    
    viewer_->draw(drawer);

    foreach (drawer_impl::point_buffer_t const & buffer, drawer.point_buffers)
    {
        qglColor(buffer.color);
        glPointSize(buffer.radius);

        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(2, GL_DOUBLE, 0, &buffer.points[0]);

        glDrawArrays(GL_POINTS, 0, buffer.points.size() / 2);

        glDisableClientState(GL_VERTEX_ARRAY);
    }

    foreach (drawer_impl::segment_buffer_t const & buffer, drawer.segment_buffers)
    {
        qglColor(buffer.color);
        
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(2, GL_DOUBLE, 0, &buffer.segments[0]);

        glDrawArrays(GL_LINES, 0, buffer.segments.size() / 2);
        glDisableClientState(GL_VERTEX_ARRAY);
    }
}

void main_window_t::wheelEvent(QWheelEvent * e)
{
    int delta = e->delta() / 8 / 15;
    if (delta > 0)
    {
        for (int i = 0; i != delta; ++i)
            zoom_ *= 1.1;
    }
    else if (delta < 0)
    {
        for (int i = 0; i != delta; --i)
            zoom_ /= 1.1;
    }
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

void main_window_t::keyReleaseEvent(QKeyEvent * event)
{
    switch (event->key())
    {
    case Qt::Key_C:
        {
            std::stringstream ss;
            ss << QInputDialog::getText(this, "center selection", "type point: ").toStdString();
            center_ = inout::read_point(ss);
            resize_impl(size().width(), size().height());
            updateGL();
            event->accept();
        }
        break;
    default:
        event->ignore();
    }
}
            

point_t main_window_t::screen_to_global(QPoint const & screen_pos) const
{
    const int w = size().width();
    const int h = size().height();
    point_t screen_size(w, h);
    point_t pos(screen_pos.x(), h - screen_pos.y());
    return center_ - (screen_size / 2 - pos) * zoom_;
}

void main_window_t::draw_string(int x, int y, const char * s)
{
    qglColor(Qt::white);
    renderText(x, y, s);
}

}

void vis_system::run_viewer(viewer_t const * viewer)
{
    main_window_t wnd(viewer);
    wnd.show();
    app->exec(); 
}
