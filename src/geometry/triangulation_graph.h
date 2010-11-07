#ifndef _TRIANGULATION_GRAPH_H_
#define _TRIANGULATION_GRAPH_H_

namespace graph
{
    struct edge_t
    {
        size_t end;
        double weight;

        edge_t( size_t e, double w )
            : end( e )
            , weight( w )
        {}
    };
}

namespace geometry
{
    namespace 
    {
        template< class Point >
        double ctg( Point const & p1, Point const & q, Point const & p2 )
        {
            double x1 = q.x() - p1.x();
            double y1 = q.y() - p1.y();
            double x2 = q.x() - p2.x();
            double y2 = q.y() - p2.y();

            double c = x1 * x2 + y1 * y2;
            double len_sqr = (x1 * x1 + y1 * y1) * (x2 * x2 + y2 * y2);
            if ( ( len_sqr - c * c ) <= 0 )
            {
                if ( c > 0 )
                    return std::numeric_limits< double >::max();
                else
                    return -std::numeric_limits< double >::max();
            }
            return c / sqrt(len_sqr - c * c);
        }

        template< class Point >
        double magic_k( Point const & p, Point const q, double l )
        {
            double x = q.x() - p.x();
            double y = q.y() - p.y();
            return (x - y) * l / (x * x + y * y);
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

    namespace WeightType
    {
        enum Type
        {
            Unweighted, Ctg, SinSum, ConstrainedSinSum
        };

        Type from_str(std::string const & str)
        {
            if (str == "unweighted")
                return Unweighted;
            if (str == "ctg")
                return Ctg;
            if (str == "sin-sum")
                return SinSum;
            if (str == "constrained-sin-sum")
                return ConstrainedSinSum;
            throw std::logic_error("There is no weight type with name = " + str);
        }
    }
    
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
}

#endif /* _TRIANGULATION_GRAPH_H_ */
