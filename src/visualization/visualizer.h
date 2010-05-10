#include <GL/freeglut.h>

namespace visualization
{
    struct device_context
    {
        virtual void draw_line ( double x1, double y1, double x2, double y2 )   = 0;
        virtual void draw_point( double x, double y, double radius )            = 0;
        virtual void set_color ( float r, float g, float b )                    = 0;
    };

    template< class Controller >
    struct visualizer
    {
        visualizer( Controller * controller, const char * name )
                : controller_( *controller )
        {
            int argc = 0;
            glutInit( &argc, NULL );
            glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA );
            glutCreateWindow( name );
            glutDisplayFunc( boost::bind( &simple_device_context::render, dc_ ) );
            glutKeyboardFunc( boost::bind( &visualizer::process_key, this, _1, _2, _3 ) );
            glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
            glutMainLoop();
        }

    private:

        void process_key( unsigned char key, int x, int y )
        {
            if ( controller_.process_key( key ) )
                glutPostRedisplay();
        }

        struct simple_device_context : device_context
        {
            void draw_line( double x1, double y1, double x2, double y2 )
            {
                line l;
                l.x1 = x1, l.y1 = y2, l.x2 = x2, l.y2 = y2;
                lines_.push_back( l );
            }

            void draw_point( double x, double y, int radius )
            {
                point p;
                p.x = x, p.y = y, p.size = radius;
                points_.push_back( p );
            }

            void set_color( float r, float g, float b )
            {
                color_event e;
                e.r = r, e.g = g, e.b = b;
                e.point_idx = points_.size(), e.line_idx = lines_.size();
                color_events_.push_back( e );
            }

            void draw()
            {
                glClear( GL_COLOR_BUFFER_BIT );
                size_t point_idx = 0, line_idx = 0;
                foreach ( color_event const & e, color_events_ )
                {
                    glColor3f( e.r, e.g, e.b );
                    glBegin( GL_LINES );
                    for ( ; line_idx != e.line_idx; ++line_idx )
                    {
                        glVertex2d( lines_[line_idx].x1, lines_[line_idx].y1 );
                        glVertex2d( lines_[line_idx].x2, lines_[line_idx].y2 );
                    }
                    glEnd();                    

                    glBegin( GL_POINTS );
                    while ( point_idx != e.point_idx )
                    {
                        GLfloat size = points_[point_idx].size;
                        glPointSize( size );
                        for ( ; ( point_idx != e.point_idx ) && ( points_[point_idx].size == size ); ++point_idx )
                            glVertex2d( points_[point_idx].x, points_[point_idx].y );
                    }
                    glEnd();
                }
            }

        private:
            struct point
            {
                double x, y;
                int size;
            };

            struct line
            {
                double x1, y1, x2, y2;
            };

            struct color_event 
            {
                float r, g, b;
                size_t line_idx, point_idx;
            };

            std::vector< point > points_;
            std::vector< line > lines_;
            std::vector< color_event > color_events_;
        };

        simple_device_context dc_;
        Controller & controller_;
    };
}
