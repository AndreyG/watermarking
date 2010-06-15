#include "viewer.h"
#include "../algorithms/util.h"

using visualization::device_context;

namespace
{
	template< class Vertex >
	void draw_vertex( device_context & dc, Vertex const & v, int radius )
	{
		dc.draw_point( v.x(), v.y(), radius );
	}
}

template< class Data >
struct my_visualizer
{
	enum DRAW
	{
		VERTICES, EDGES, TRIANGULATION, SPLIT_LINES, SUBAREA_VERTICES, MODIFIED_VERTICES, DRAW_SIZE
	};

	my_visualizer( Data * data )
			: data_( *data )
	{
		for ( size_t i = 0; i != DRAW_SIZE; ++i )
			draw_[i] = false;
		key2draw_['e'] = EDGES;
		key2draw_['v'] = VERTICES;
		key2draw_['s'] = SPLIT_LINES;
		key2draw_['p'] = SUBAREA_VERTICES;
        key2draw_['m'] = MODIFIED_VERTICES;
        key2draw_['t'] = TRIANGULATION;

		current_subarea_ = -1;
	}

	void draw( device_context & dc ) const
	{
		//        std::cout << "draw vertices = " << draw_[VERTICES]  << std::endl;
		//        std::cout << "draw edges = "    << draw_[EDGES]     << std::endl;
		//        std::cout << "draw split lines = " << draw_[SPLIT_LINES] << std::endl;
		vertices_t const & vertices = data_.graph_.vertices;

		if ( draw_[VERTICES] )
		{
			dc.set_color( 0, 1, 0 );
			foreach ( typename vertices_t::value_type const & v, vertices )
				draw_vertex( dc, v, 2 );
		}
		if ( draw_[MODIFIED_VERTICES] )
		{
			dc.set_color( 0.7, 0.7, 0 );
            foreach ( vertices_t const & vertices, data_.modified_vertices_ )
			    foreach ( typename vertices_t::value_type const & v, vertices )
				    draw_vertex( dc, v, 2 );
		}
		if ( draw_[SUBAREA_VERTICES] )
		{
            using algorithm::find_last;

            typedef typename vertices_t::const_iterator iter_t;
            typedef std::vector< size_t > indices_t;

            indices_t const & subdivision = data_.subdivision_;
            iter_t p = vertices.begin() + std::distance( subdivision.begin(), std::find( subdivision.begin(), subdivision.end(), current_subarea_ ) );
            iter_t q = vertices.begin() + std::distance( subdivision.begin(), find_last( subdivision.begin(), subdivision.end(), current_subarea_ ) ) + 1; 
			
            dc.set_color( 1, 0, 0 );
            draw_split_lines( dc, p, q );

			dc.set_color( 0, 0.7, 0.7 );
            for ( ; p != q; ++p )
			{
			    draw_vertex( dc, *p, 4 );
			}
		}
        if ( draw_[EDGES] )
        {
            dc.set_color( 0, 1, 0 );
            typedef typename Data::graph_t::edge_t edge_t; 
            foreach ( edge_t const & e, data_.graph_.edges )
            {
                typename Data::graph_t::vertex_t const & v1 = vertices[e.first];
                typename Data::graph_t::vertex_t const & v2 = vertices[e.second];
                dc.draw_line( v1.x(), v1.y(), v2.x(), v2.y() );
            }
        }
		if ( draw_[TRIANGULATION] )
		{
			dc.set_color( 1, 0, 0 );
			draw_triangulation( dc );
		}
		if ( draw_[SPLIT_LINES] )
		{
			dc.set_color( 0, 0, 1 );
			draw_split_lines( dc );
		}
	}

	bool process_key( unsigned char key )
	{
		switch ( key )
		{
		case 'n':
			data_.next_step();
			return true;
		case 'f':
			++current_subarea_;
			if ( current_subarea_ == data_.graph_.vertices.size() )
				current_subarea_ = -1;
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
	void draw_triangulation( device_context & dc ) const
	{
		typedef typename Data::trg_t trg_t;
		typedef typename trg_t::Finite_vertices_iterator vertices_iterator;
		typedef typename trg_t::Finite_edges_iterator edges_iterator;
		typedef typename trg_t::Vertex_handle vertex_handle;
		typedef typename trg_t::Face_handle face_handle;

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

    template< class Iter >
    void draw_split_lines( device_context & dc, Iter p, Iter q ) const
    {
		using algorithm::make_min;
		using algorithm::make_max;

        double min_x = std::numeric_limits< double >::max();
		double min_y = std::numeric_limits< double >::max();
		double max_x = -min_x;
		double max_y = -min_y;

		for ( ; p != q; ++p )
		{
            make_min( min_x, p->x() );
		    make_max( max_x, p->x() );
		    make_min( min_y, p->y() );
			make_max( max_y, p->y() );
	    }
	    dc.draw_line( min_x, min_y, min_x, max_y );
		dc.draw_line( max_x, min_y, max_x, max_y );
		dc.draw_line( min_x, min_y, max_x, min_y );
		dc.draw_line( min_x, max_y, max_x, max_y );
    }

	void draw_split_lines( device_context & dc ) const
	{
		if ( data_.step_ == Data::SUBDIVIDE_PLANE )
			return;

        typename vertices_t::const_iterator p = data_.graph_.vertices.begin();
		for ( size_t s = 0, i = 0; s != data_.subareas_num_; ++s )
		{
            while ( ( i != data_.subdivision_.size() ) && ( data_.subdivision_[i] == s ) )
                ++i;
            typename vertices_t::const_iterator q = data_.graph_.vertices.begin() + i;
            draw_split_lines( dc, p, q );
            p = q;
		}
	}

private:
	typedef typename Data::graph_t::vertices_t vertices_t;

	Data & data_;

	bool draw_[DRAW_SIZE];
	std::map< unsigned char, size_t > key2draw_;

	size_t current_subarea_;
};
