#include "viewer.h"
#include "../algorithms/util.h"

using visualization::device_context;

template< class Data >
struct my_visualizer
{
    enum DRAW
    {
        VERTICES, EDGES, SPLIT_LINES, DRAW_SIZE
    };

    my_visualizer( Data * data )
            : data_( *data )
    {
        for ( size_t i = 0; i != DRAW_SIZE; ++i )
            draw_[i] = false;
        key2draw_['e'] = EDGES;
        key2draw_['v'] = VERTICES;
        key2draw_['s'] = SPLIT_LINES;
    }

    void draw( device_context & dc ) const
    {
        typedef typename Data::trg_t trg_t;
        typedef typename trg_t::Finite_vertices_iterator vertices_iterator;
        typedef typename trg_t::Finite_edges_iterator edges_iterator;
        typedef typename trg_t::Vertex_handle vertex_handle;
        typedef typename trg_t::Face_handle face_handle;

        std::cout << "draw vertices = " << draw_[VERTICES]  << std::endl;
        std::cout << "draw edges = "    << draw_[EDGES]     << std::endl;
        std::cout << "draw split lines = " << draw_[SPLIT_LINES] << std::endl;
        if ( draw_[VERTICES] )
        {
            dc.set_color( 0, 1, 0 );
            foreach ( trg_t const & trg, data_.trgs_ )
            {
                for ( vertices_iterator v = trg.vertices_begin(); v != trg.vertices_end(); ++v )
                {
                    dc.draw_point( v->point().x(), v->point().y(), 2 );
                }
            }
        }
        if ( draw_[EDGES] ) 
        {
            dc.set_color( 1, 0, 0 );
            foreach ( trg_t const & trg, data_.trgs_ )
            {
                for ( edges_iterator e = trg.edges_begin(); e != trg.edges_end(); ++e )
                {
                    int v = e->second;
                    face_handle f = e->first;
                    vertex_handle v1 = f->vertex( f->cw( v ) );
                    vertex_handle v2 = f->vertex( f->ccw( v ) );
                    dc.draw_line( v1->point().x(), v1->point().y(), v2->point().x(), v2->point().y() );
                }
            }
        }
        if ( draw_[SPLIT_LINES] )
        {
        	dc.set_color( 0, 0, 1 );

        	using algorithm::make_min;
        	using algorithm::make_max;

        	for ( size_t s = 0, i = 0; s != data_.subareas_num_; ++s )
        	{
        		double min_x = std::numeric_limits< double >::max();
        		double min_y = std::numeric_limits< double >::max();
        		double max_x = std::numeric_limits< double >::min();
        		double max_y = std::numeric_limits< double >::min();

        		for ( ; i != data_.subdivision_.size() && data_.subdivision_[i] == s; ++i )
        		{
        			make_min( min_x, data_.graph_.vertices[i].x() );
        			make_min( min_y, data_.graph_.vertices[i].y() );
        			make_max( max_x, data_.graph_.vertices[i].x() );
        			make_max( max_y, data_.graph_.vertices[i].y() );
        		}
        		dc.draw_line( min_x, min_y, min_x, max_y );
        		dc.draw_line( max_x, min_y, max_x, max_y );
        		dc.draw_line( min_x, min_y, max_x, min_y );
        		dc.draw_line( min_x, max_y, max_x, max_y );
        	}
        }
    }

    bool process_key( unsigned char key ) 
    {
    	if ( key == 'n' )
    	{
    		data_.next_step();
    		return true;
    	}
        std::map< unsigned char, size_t >::const_iterator it = key2draw_.find( key );
        if ( it != key2draw_.end() )
        {
            draw_[it->second] = !draw_[it->second];
            return true;
        }
        return false;
    }

private:
    Data & data_;
    bool draw_[DRAW_SIZE];
    std::map< unsigned char, size_t > key2draw_;
};
