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

    template< class FwdIter >
    void draw_vertices( device_context & dc, FwdIter p, FwdIter q, float r, float g, float b, int radius )
    {
        dc.set_color( r, g, b );
        for( ; p != q; ++p )
            draw_vertex( dc, *p, radius );
    }

    template< class Container >
    void draw_vertices( device_context & dc, Container const & vertices, float r, float g, float b, int radius )
    {
        draw_vertices( dc, vertices.begin(), vertices.end(), r, g, b, radius );
    }
}

template< class Data >
struct my_visualizer
{
private:
	typedef typename Data::graph_t::vertices_t  vertices_t;
    typedef typename Data::graph_t::edges_t     edges_t;

	enum DRAW
	{
		VERTICES, EDGES, TRIANGULATION, SPLIT_LINES, SUBAREA_VERTICES, 
        MODIFIED_VERTICES, MODIFIED_EDGES, 
        NOISED_VERTICES, NOISED_EDGES, DRAW_SIZE
	};

public:
	my_visualizer( Data * data );

	void draw( device_context & dc ) const;

	bool process_key( unsigned char key );

private:
	void draw_triangulation( device_context & dc ) const;

    void draw_edges( device_context & dc, vertices_t const & vertices, edges_t const & edges ) const
    {
        foreach ( typename edges_t::value_type const & e, edges )
        {
            dc.draw_line(   vertices[e.first].x(), vertices[e.first].y(), 
                            vertices[e.second].x(), vertices[e.second].y() );
        }
    }

    template< class Iter >
    void draw_split_lines( device_context & dc, Iter p, Iter q ) const;

	void draw_split_lines( device_context & dc ) const;

private:

	Data & data_;
    typename Data::graph_t noised_graph_;

	bool draw_[DRAW_SIZE];
	std::map< unsigned char, size_t > key2draw_;

	size_t current_subarea_;
};

template< class Data >
my_visualizer< Data>::my_visualizer( Data * data )
			: data_( *data )
{
    for ( size_t i = 0; i != DRAW_SIZE; ++i )
	    draw_[i] = false;

	key2draw_['e'] = EDGES;
	key2draw_['v'] = VERTICES;
	key2draw_['s'] = SPLIT_LINES;
	key2draw_['p'] = SUBAREA_VERTICES;
    key2draw_['m'] = MODIFIED_VERTICES;
    key2draw_['o'] = MODIFIED_EDGES;
    key2draw_['t'] = TRIANGULATION;
    key2draw_['q'] = NOISED_VERTICES;
    key2draw_['w'] = NOISED_EDGES;

    current_subarea_ = -1;
}

template< class Data >
void my_visualizer< Data >::draw( device_context & dc ) const
{
	vertices_t const & vertices = data_.graph_.vertices;

    if ( draw_[VERTICES] )
	{
        draw_vertices( dc, vertices, 0, 1, 0, 2 );
	}
    if ( draw_[EDGES] )
    {
        dc.set_color( 0, 1, 0 );
        draw_edges( dc, vertices, data_.graph_.edges );
    }
    if ( draw_[NOISED_VERTICES] )
    {
        draw_vertices( dc, noised_graph_.vertices, 0, 0.7, 0.7, 2 );
    }
    if ( draw_[NOISED_EDGES] )
    {
        dc.set_color( 0, 0.7, 0.7 );
        draw_edges( dc, noised_graph_.vertices, noised_graph_.edges );
    }
	if ( draw_[SUBAREA_VERTICES] && ( data_.step_ > Data::SUBDIVIDE_PLANE ) )
	{
        using algorithm::find_last;

        typedef typename vertices_t::const_iterator iter_t;
        typedef std::vector< size_t > indices_t;

        indices_t const & subdivision = data_.subdivision_;
        iter_t begin = vertices.begin();
        iter_t p = begin + std::distance( subdivision.begin(), std::find( subdivision.begin(), subdivision.end(), current_subarea_ ) );
        iter_t q = begin + std::distance( subdivision.begin(), find_last( subdivision.begin(), subdivision.end(), current_subarea_ ) ) + 1; 
			
        dc.set_color( 1, 0, 0 );
        draw_split_lines( dc, p, q );

        draw_vertices( dc, p, q, 0, 0.7, 0.7, 4 );
	}
	if ( draw_[MODIFIED_VERTICES] && ( data_.step_ > Data::MODIFY_VERTICES ) )
	{
        draw_vertices( dc, data_.modified_vertices_, 0.7, 0.7, 0, 2 );
	}
    if ( draw_[MODIFIED_EDGES] && ( data_.step_ > Data::MODIFY_VERTICES ) )
    {
        dc.set_color( 0.7, 0.7, 0 );
        draw_edges( dc, data_.modified_vertices_, data_.graph_.edges );
    }
	if ( draw_[TRIANGULATION] && ( data_.step_ > Data::BUILD_TRIANGULATIONS ) )
	{
	    dc.set_color( 1, 0, 0 );
		draw_triangulation( dc );
	}
	if ( draw_[SPLIT_LINES] && ( data_.step_ > Data::SUBDIVIDE_PLANE ) )
	{
		dc.set_color( 0, 0, 1 );
		draw_split_lines( dc );
	}
}

template< class Data >
void my_visualizer< Data >::draw_triangulation( device_context & dc ) const
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

template< class Data, class Iter >
void my_visualizer< Data >::draw_split_lines( device_context & dc, Iter p, Iter q ) const
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

template< class Data >
void my_visualizer< Data >::draw_split_lines( device_context & dc ) const
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

template< class Data >
bool my_visualizer< Data >::process_key( unsigned char key )
{
    switch ( key )
	{
	case 'n':
	    return data_.next_step();
	case 'f':
		++current_subarea_;
		if ( current_subarea_ == data_.graph_.vertices.size() )
			current_subarea_ = -1;
		return true;
    case 'a':
        {
            namespace po = boost::program_options;

            std::ifstream conf("noise.conf");
                
            po::options_description desc;
            desc.add_options()
                ( "epsilon",   po::value< double >() )
            ;

            po::variables_map vm;       
            po::store( po::parse_config_file( conf, desc ), vm );

            noised_graph_ = watermarking::add_noise( data_.modified_graph(), vm["epsilon"].as< double >() );
        } 
        return false;
    case 'r':
        if ( data_.step_ < Data::MODIFY_VERTICES )
            return false;
        {
            namespace po = boost::program_options;

            std::ifstream conf("embedding.conf");
                
            po::options_description desc;
            desc.add_options()
                ( "message", po::value< std::string >() )
                ( "key",     po::value< int >() )
                ( "chip-rate", po::value< size_t >() )
                ( "alpha",   po::value< double >() )
            ;

            po::variables_map vm;       
            po::store( po::parse_config_file( conf, desc ), vm );

            std::string message_text = vm["message"].as< std::string >();
            std::cout << message_text << std::endl;
    
            watermarking::message_t message( message_text.size() * 7 );
            for ( size_t i = 0; i != message_text.size(); ++i )
            {
                unsigned char c = message_text[i];
                for ( size_t j = 0; j != 7; ++j )
                {
                    message[i * 7 + j] = c % 2;
                    c /= 2;
                }
            }
    
            data_.modify_vertices( message, vm["chip-rate"].as< size_t >(),
                                            vm["key"].as< int >(),
                                            vm["alpha"].as< double >() );

            return true;
        }
    case 'g':
        {
            watermarking::message_t message = watermarking::extract( ... );

            for ( size_t i = 0; i != message.size(); )
            {
                unsigned char c = 0;
                int pow = 1;
                for ( size_t j = 0; j != 7; ++j, ++i, pow *= 2 )
                {
                    c += message[i] * pow;
                }
                std::cout << c;
            }
            std::cout << std::endl;
        }
    }

	std::map< unsigned char, size_t >::const_iterator it = key2draw_.find( key );
	if ( it != key2draw_.end() )
	{
		draw_[it->second] = !draw_[it->second];
		return true;
	}
	return false;
}

