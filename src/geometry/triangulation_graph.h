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
    }
    
    template< class Triangulation >
    struct triangulation_graph
    {
    private:
        typedef std::vector< graph::edge_t >            edges_t;
        typedef typename Triangulation::Vertex_handle   vertex_t;
            
    public:
        typedef edges_t::const_iterator edges_iterator;

        triangulation_graph( Triangulation const & trg, std::map< vertex_t, size_t > & index, bool weighted)
                    : edges_( index.size() )
                    , degree_( index.size(), 0 )
        {
            if ( !weighted )
            {
                typedef typename Triangulation::Finite_vertices_iterator    vertices_iterator;
                typedef typename Triangulation::Vertex_circulator           vertex_circulator;
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
            }
            else
            {
                typedef typename Triangulation::Finite_edges_iterator edges_iterator;
                typedef typename Triangulation::Face_handle face_t;
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
                            if ( w < -1e-5 )
                            {
                                std::cout << w1 << "\t" << w2 << std::endl;
                            }
                            assert( w >= -1e-5 );
                        }
                    }
                    if ( w > 1e-3 )
                    {   
                        edges_[index[b]].push_back( graph::edge_t( index[c], w ) );
                        edges_[index[c]].push_back( graph::edge_t( index[b], w ) );
                    }
                }
            }
            for ( size_t v = 0; v != degree_.size(); ++v )
            {
                foreach ( graph::edge_t const & e, edges_[v] )
                    degree_[v] += e.weight;
                assert( degree_[v] > 0 );
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
