#ifndef _TRIANGULATION_GRAPH_H_
#define _TRIANGULATION_GRAPH_H_

namespace geometry
{
    namespace 
    {
        double ctg( point_t p1, point_t const & q, point_t p2 )
        {
            p1 -= q;
            p2 -= q;

            double c = p1 * p2;
            double s = p1 ^ p2;
            if ( s == 0 )
            {
                if ( c > 0 )
                    return std::numeric_limits< double >::max();
                else
                    return -std::numeric_limits< double >::max();
            }
            return c / s;
        }

        double magic_k( point_t const & p, point_t const & q, double l )
        {
            point_t v = q - p;
            return ( v.x() - v.y() ) * l / mod( v );
        }

        template< class Point >
        double weight( Point const & p1, Point const & q, Point const & p2, double l )
        {
            double k_p1 = magic_k( q, p2, l );
            double k_q = magic_k( p2, p1, l );
            double k_p2 = magic_k( p1, q, l );

            return k_q * (2 * k_q - (k_p1 + k_p2));
        }
    }

	/*
    template< class Triangulation >
    struct triangulation_graph
    {
    private:
        typedef std::vector< graph::edge_t >            edges_t;
        typedef typename Triangulation::Vertex_handle   vertex_t;
            
    public:
        typedef edges_t::const_iterator edges_iterator;

        triangulation_graph(    Triangulation const & trg, std::map< vertex_t, size_t > & index, 
                                bool is_constrained, WeightType::Type type )
                    : edges_( index.size() )
                    , degree_( index.size(), 0 )
        {
            typedef typename Triangulation::Finite_vertices_iterator    vertices_iterator;
            typedef typename Triangulation::Vertex_circulator           vertex_circulator;
            typedef typename Triangulation::Finite_edges_iterator       edges_iterator;
            typedef typename Triangulation::Face_handle                 face_t;
            typedef typename Triangulation::Face_circulator             face_circulator;
            switch ( type )
            {
            case WeightType::Unweighted:
                for ( vertices_iterator v = trg.finite_vertices_begin(); v != trg.finite_vertices_end(); ++v )
                {
                    edges_t & edges = edges_[index[v]];
                    vertex_circulator vc_begin = trg.incident_vertices( v ), vc( vc_begin );
                    do 
                    {
                        if ( !trg.is_infinite( vc ) )
                            edges.push_back( graph::edge_t( index[vc], 1 ) );
                    } while ( ++vc != vc_begin );
                }
                for ( size_t v = 0; v != degree_.size(); ++v )
                {
                    degree_[v] = edges_[v].size();
                }
                break;
            case WeightType::Ctg:
                for ( edges_iterator e = trg.finite_edges_begin(); e != trg.finite_edges_end(); ++e )
                {
                    face_t f = e->first;
                    vertex_t a = f->vertex( e->second );
                    vertex_t b = f->vertex( trg.cw( e->second ) );
                    vertex_t c = f->vertex( trg.ccw( e->second ) );
                    face_t neighbor = f->neighbor( e->second ); 
                
                    assert( !trg.is_infinite( a ) || !trg.is_infinite( neighbor ) );

                    double w = -1;
                    if ( trg.is_infinite( neighbor ) )
                    {
                        w = ctg( b->point(), a->point(), c->point() ) / 2;
                    }   
                    else 
                    {
                        vertex_t d;
                        for ( int i = 0; i != 3; ++i )
                        {
                            d = neighbor->vertex( i );
                            if ( ( d->point() != b->point() ) && ( d->point() != c->point() ) )
                            {
                                break;
                            }
                        }
                        if ( trg.is_infinite( a ) )
                        { 
                            w = ctg( b->point(), d->point(), c->point() );
                            if ( w < 0 )
                                w = 1;
                        }
                        else
                        {
                            double w1 = ctg( b->point(), a->point(), c->point() );
                            double w2 = ctg( b->point(), d->point(), c->point() ); 
                            w = ( w1 + w2 ) / 2.0;
                            if (!is_constrained )
                            {
                                if ( w < -1e-5 )
                                {
                                    std::cout << w1 << "\t" << w2 << std::endl;
                                }
                                assert( w >= -1e-5 );
                            }
                        }
                    }
                    if ( (!is_constrained && (w > 1e-3)) || (is_constrained && (abs(w) > 1e-3)) )
                    {   
                        edges_[index[b]].push_back( graph::edge_t( index[c], w ) );
                        edges_[index[c]].push_back( graph::edge_t( index[b], w ) );
                    }
                }
                for ( size_t v = 0; v != degree_.size(); ++v )
                {
                    foreach ( graph::edge_t const & e, edges_[v] )
                        degree_[v] += e.weight;
                    if (!is_constrained)
                        assert( degree_[v] > 0 );
                }    
                break;
            case WeightType::SinSum:
            case WeightType::ConstrainedSinSum:
                double mean_length = 0;
                size_t edges_num = 0;
                for ( edges_iterator e = trg.finite_edges_begin(); e != trg.finite_edges_end(); ++e )
                {
                    if ( (type == WeightType::ConstrainedSinSum) && !trg.is_constrained(*e) )
                        continue;
                    face_t f = e->first;
                    auto p = f->vertex( trg.cw( e->second ) )->point();
                    auto q = f->vertex( trg.ccw( e->second ) )->point();
                    double x = q.x() - p.x();
                    double y = q.y() - p.y();
                    mean_length += sqrt(x * x + y * y);
                    ++edges_num;
                }
                mean_length /= edges_num;
                for ( edges_iterator e = trg.finite_edges_begin(); e != trg.finite_edges_end(); ++e )
                {
                    if ( (type == WeightType::ConstrainedSinSum) && !trg.is_constrained(*e) )
                        continue;
                    face_t f = e->first;
                    vertex_t a = f->vertex( e->second );
                    vertex_t b = f->vertex( trg.cw( e->second ) );
                    vertex_t c = f->vertex( trg.ccw( e->second ) );
                    face_t neighbor = f->neighbor( e->second ); 
                
                    assert( !trg.is_infinite( a ) || !trg.is_infinite( neighbor ) );

                    double w;
                    if ( trg.is_infinite( neighbor ) )
                    {
                        w = weight( b->point(), a->point(), c->point(), mean_length );
                    }   
                    else 
                    {
                        vertex_t d;
                        for ( int i = 0; i != 3; ++i )
                        {
                            d = neighbor->vertex( i );
                            if ( ( d->point() != b->point() ) && ( d->point() != c->point() ) )
                            {
                                break;
                            }
                        }
                        if ( trg.is_infinite( a ) )
                        { 
                            w = ctg( b->point(), d->point(), c->point() );
                        }
                        else
                        {
                            double w1 = weight( b->point(), a->point(), c->point(), mean_length );
                            double w2 = weight( b->point(), d->point(), c->point(), mean_length ); 
                            w = w1 + w2;
                        }
                    }
                    if ( abs(w) > 1e-3 )
                    {   
                        edges_[index[b]].push_back( graph::edge_t( index[c], w ) );
                        edges_[index[c]].push_back( graph::edge_t( index[b], w ) );
                    }
                }
                for ( size_t v = 0; v != degree_.size(); ++v )
                {
                    foreach ( graph::edge_t const & e, edges_[v] )
                        degree_[v] += e.weight;
                }    
                for ( vertices_iterator v = trg.finite_vertices_begin(); v != trg.finite_vertices_end(); ++v )
                {
                    double & deg = degree_[index[v]];
                    face_circulator fc_begin = trg.incident_faces(v), fc = fc_begin;
                    do
                    {
                        if ( !trg.is_infinite( fc ) )
                        {
                            int i = 0;
                            for ( ; i != 3; ++i )
                            {
                                if ( fc->vertex( i ) == (vertex_t) v )
                                    break;
                            }
                            if ((type != WeightType::ConstrainedSinSum) || fc->is_constrained(i)) 
                            {
                                double k_z = magic_k( fc->vertex( trg.cw( i ) )->point(), fc->vertex( trg.ccw( i ) )->point(), mean_length ); 
                                double k_x = magic_k( v->point(), fc->vertex( trg.cw( i ) )->point(), mean_length );
                                double k_y = magic_k( fc->vertex( trg.ccw( i ) )->point(), v->point(), mean_length );
                                deg += k_z * ( k_x + k_y );
                            }
                        }
                    } while ( ++fc != fc_begin );
                }
            }
        }

        size_t vertices_num() const
        {
            return edges_.size();
        }

        double degree( size_t v ) const
        {
            return degree_[v];
        }

        edges_iterator edges_begin( size_t v ) const
        {
            return edges_[v].begin();
        }

        edges_iterator edges_end( size_t v ) const
        {
            return edges_[v].end();
        }

    private:
        std::vector< edges_t > edges_; 
        std::vector< double > degree_;
    };
	*/

	template< class Triangulation >
	struct triangulation_graph
	{
		struct edge_t
		{
			size_t b, e;
			size_t left, right;
		};

	private:
		typedef typename Triangulation::Vertex_handle				vertex_t;
		typedef typename Triangulation::Face_handle					face_t;

	public:
		explicit triangulation_graph( Triangulation const & trg )
		{
            typedef typename Triangulation::Finite_vertices_iterator    vertices_iterator;
            typedef typename Triangulation::Finite_edges_iterator       edges_iterator;

			std::map< vertex_t, size_t > v2i;
			size_t i = 0;
			for ( vertices_iterator v = trg.finite_vertices_begin(); v != trg.finite_vertices_end(); ++v, ++i )
			{
				v2i[v] = i;
				vertices_.push_back( point_t(v->point()) );
			}

			for ( edges_iterator e = trg.finite_edges_begin(); e != trg.finite_edges_end(); ++e )
			{
				face_t f = e->first;
				const int i = e->second;

				edge_t edge;
				vertex_t v[] = 
				{
					f->vertex( trg.ccw( i ) ),					 
					f->vertex( trg.cw( i ) )
				};

				edge.b = v2i[v[0]];
				edge.e = v2i[v[1]];

				if ( trg.is_infinite( f ) )
					edge.left = vertices_num();
				else
					edge.left = v2i[vertex(f, v)];
				f = f->neighbor( i );
				if ( trg.is_infinite( f ) )
					edge.right = vertices_num();
				else
					edge.right = v2i[vertex(f, v)];
				
				assert(edge.left < vertices_num() || edge.right < vertices_num());
				edges_.push_back( edge );
			}
		}
	
	private:
		
		vertex_t vertex(face_t const & f, vertex_t const * v) const
		{
			for (int i = 0; i != 3; ++i)
			{
				vertex_t u = f->vertex(i);
				if ((u != v[0]) && (u != v[1]))
					return u;
			}
			throw std::logic_error("every vertex in the face is included in its neighbor");
		}

	public:

		point_t const & vertex( size_t v ) const
		{
			return vertices_[v];
		}

		edge_t const & edge( size_t e ) const
		{
			return edges_[e];
		}

		size_t vertices_num() const
		{
			return vertices_.size();
		}

		size_t edges_num() const
		{
			return edges_.size();
		}

	private:
		std::vector< point_t > 	vertices_;
		std::vector< edge_t > 	edges_;
	};
}

#endif /* _TRIANGULATION_GRAPH_H_ */
