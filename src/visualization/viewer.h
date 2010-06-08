#include <GL/freeglut.h>

namespace visualization
{
    struct device_context
    {
        virtual void draw_line ( double x1, double y1, double x2, double y2 )   = 0;
        virtual void draw_point( double x, double y, int radius )               = 0;
        virtual void set_color ( float r, float g, float b )                    = 0;
    };

    namespace details
    {
        void* fake_viewer;

        template< class Viewer >
        void display_func()
        {
            std::cout << "visualization::details::display_func\n";
            ( (Viewer *) fake_viewer )->render();
        }

        double zoom = 1.0;

        double width = 1000, height = 1000;
        double center_x = 0, center_y = 0;

        void change_size( GLsizei screen_w, GLsizei screen_h )
        {
            std::cout << "visualization::details::change_size\n";
            if ( screen_h == 0 )
                screen_h = 1;
            glViewport( 0, 0, screen_w, screen_h );
            glMatrixMode( GL_PROJECTION );
            glLoadIdentity();
            double aspect_ratio = 1.0 * screen_w / screen_h;
            double w = zoom * width;
            double h = zoom * height;
            if ( w <= h )
                h /= aspect_ratio;
            else
                w /= aspect_ratio;
            gluOrtho2D( -w + center_x, w + center_x, -h + center_y, h + center_y );
            glMatrixMode( GL_MODELVIEW );
            glLoadIdentity();
        }

        void change_zoom( bool inc )
        {
            if ( inc )
            {
                zoom *= 0.9;
            }
            else
            {
                zoom /= 0.9;
            }
            std::cout << "zoom = " << zoom << std::endl;
            int screen_width  = glutGet( GLUT_WINDOW_WIDTH  );
            int screen_height = glutGet( GLUT_WINDOW_HEIGHT );
            change_size( screen_width, screen_height );
            glutPostRedisplay();
        }

        template< class Viewer >
        void keyboard_func( unsigned char key, int x, int y )
        {
            switch ( key )
            {
            case '+':
                change_zoom( true );
                break;
            case '-':
                change_zoom( false );
                break;
            default:
                ( (Viewer *) fake_viewer )->process_key( key, x, y );
            }
        }

        void mouse_wheel( int button, int dir, int x, int y )
        {
            change_zoom( dir > 0 );
        }         

    }

    template< class Controller >
    struct viewer
    {
        typedef viewer< Controller > self_t;

        viewer( Controller * controller, const char * name, int* argcp, char** argv )
                : controller_( *controller )
        {
            glutInit( argcp, argv );
            glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA );
            glutCreateWindow( name );

            details::fake_viewer = this;
            glutDisplayFunc( &details::display_func< self_t > ); 
            glutKeyboardFunc( &details::keyboard_func< self_t > ); 
            glutReshapeFunc( &details::change_size );
            glutMouseWheelFunc( &details::mouse_wheel );

            glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
            glutMainLoop();
        }

        void process_key( unsigned char key, int x, int y )
        {
            if ( controller_.process_key( key ) )
                glutPostRedisplay();
        }

        void render()
        {
            dc_.clear();
            controller_.draw( dc_ );
            dc_.render();
        }

    private:

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

            void clear()
            {
                points_.clear();
                lines_.clear();
                color_events_.clear();
            }

            void render()
            {
                std::cout << "simple_device_context::render, points_num = " << points_.size() <<"\tlines_num = " << lines_.size() << std::endl;
                std::cout << "events_num = " << color_events_.size() << std::endl;
                glClear( GL_COLOR_BUFFER_BIT );
                glColor3f( 1.0f, 1.0f, 1.0f );
                if ( color_events_.empty() )
                    render_primitives( 0, points_.size(), 0, lines_.size() );
                else
                {
                    render_primitives( 0, color_events_.front().point_idx, 0, color_events_.front().line_idx );
                    for ( size_t i = 0, j = 1; j != color_events_.size(); ++i, ++j )
                    {
                        color_event const & e1 = color_events_[i];
                        color_event const & e2 = color_events_[j];
                        glColor3f( e1.r, e1.g, e1.b );
                        render_primitives( e1.point_idx, e2.point_idx, e1.line_idx, e2.line_idx );
                    }
                    color_event const & e = color_events_.back(); 
                    glColor3f( e.r, e.g, e.b );
                    render_primitives( e.point_idx, points_.size(), e.line_idx, lines_.size() );
                }
                glutSwapBuffers();
            }

        private:
            void render_primitives( size_t point_idx, size_t point_end, size_t line_idx, size_t line_end )
            {
                glBegin( GL_LINES );
                for ( ; line_idx != line_end; ++line_idx )
                {
                    glVertex2d( lines_[line_idx].x1, lines_[line_idx].y1 );
                    glVertex2d( lines_[line_idx].x2, lines_[line_idx].y2 );
                }
                glEnd();                    

                glBegin( GL_POINTS );
                while ( point_idx != point_end )
                {
                    GLfloat size = points_[point_idx].size;
                    glPointSize( size );
                    std::cout << "points with size " << size << " num = " << point_end - point_idx << std::endl;
                    for ( ; ( point_idx != point_end ) && ( points_[point_idx].size == size ); ++point_idx )
                        glVertex2d( points_[point_idx].x, points_[point_idx].y );
                }
                glEnd();
            }

            void scale_x( double x )
            {

            }

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
