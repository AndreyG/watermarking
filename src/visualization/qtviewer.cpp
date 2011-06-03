#include "../stdafx.h"
#include "qtviewer.h"

#include "../utility/stopwatch.h"
#include "../utility/debug_stream.h"
#include "../geometry/point.h"
#include "drawer_impl.h"
#include "printer_impl.h"

namespace visualization
{

stream_t::~stream_t() {}
drawer_t::~drawer_t() {}
printer_t::~printer_t() {}
viewer_t::~viewer_t() {}

boost::scoped_ptr<QApplication> vis_system::app;

void vis_system::init(int argc, char ** argv)
{
    app.reset(new QApplication(argc, argv));
    new QGLWidget;
}

namespace
{
using geometry::point_t;

struct main_window_t : QGLWidget
{
public:
    explicit main_window_t(viewer_t * viewer);
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

private:
    void draw_string(int x, int y, const char * s);
    void draw_string_global(double x, double y, const char * s);

private:
    viewer_t * viewer_;

    point_t center_;
    point_t size_;
    point_t current_pos_;
    double  zoom_;
    boost::optional<point_t> start_point_;
    drawer_impl     drawer_;
};

main_window_t::main_window_t(viewer_t * viewer)
    : viewer_(viewer)
    , size_(100, 100)
    , current_pos_(center_)
    , zoom_(1.0)
{
    setMouseTracking(true);
    viewer_->draw(drawer_);
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
    
    foreach (drawer_impl::point_buffer_t const & buffer, drawer_.point_buffers)
    {
        glPointSize(buffer.radius);

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
        
        glVertexPointer (2, GL_DOUBLE, 0, &buffer.points[0]);
        glColorPointer  (3, GL_DOUBLE, 0, &buffer.colors[0]);

        glDrawArrays(GL_POINTS, 0, buffer.points.size() / 2);

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);
    }

    foreach (drawer_impl::segment_buffer_t const & buffer, drawer_.segment_buffers)
    {
        glLineWidth(buffer.width);
        
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);

        glVertexPointer (2, GL_DOUBLE, 0, &buffer.segments[0]);
        glColorPointer  (3, GL_DOUBLE, 0, &buffer.colors[0]);

        glDrawArrays(GL_LINES, 0, buffer.segments.size() / 2);
        
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);
    }

    printer_impl printer(   boost::bind(&main_window_t::draw_string, this, _1, _2, _3),
                            boost::bind(&main_window_t::draw_string_global, this, _1, _2, _3));

    *printer.corner_stream() << "Mouse pos: " << current_pos_;
    viewer_->print(printer);
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
    updateGL();
}

void main_window_t::mouseReleaseEvent(QMouseEvent * )
{
    if (start_point_)
    {
        center_ -= (current_pos_ - *start_point_);
        start_point_ = boost::none;
        resize_impl(size().width(), size().height());
        updateGL();
    }
}

void main_window_t::keyReleaseEvent(QKeyEvent * event)
{
    switch (event->key())
    {
    case Qt::Key_C:
        {
            std::stringstream ss;
            ss << QInputDialog::getText(this, "center selection", "type point: ").toStdString();
            ss >> center_;
            resize_impl(size().width(), size().height());
            updateGL();
        }
        break;
    default:
        if (viewer_->on_key(event->key()))
        {
            drawer_.clear();
            viewer_->draw(drawer_);
            updateGL();
        }
    }
    event->accept();
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

void main_window_t::draw_string_global(double x, double y, const char * s)
{
    qglColor(Qt::white);
    renderText(x, y, 0, s);
}

}

void vis_system::run_viewer(viewer_t * viewer)
{
    main_window_t * wnd = new main_window_t(viewer);
    wnd->show();
    app->exec(); 
}

}
