namespace visualization
{
    struct device_context
    {
        void draw_line( double x1, double y1, double x2, double y2 );
        void draw_pixel( double x, double y, double radius );
    };

    struct visualizer
    {
        typedef boost::function< void ( device_contex & ) > render_func;
        
        visualizer( render_func & );
    };
}
