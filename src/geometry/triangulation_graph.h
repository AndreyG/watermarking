#ifndef _TRIANGULATION_GRAPH_H_
#define _TRIANGULATION_GRAPH_H_

namespace graph
{
    struct edge_t
    {
        double weight;
        size_t end;

        edge_t( size_t e, double w )
            : end( e )
            , weight( w )
        {}
    };
}

namespace geometry
{
    template< class Triangulation >
    struct triangulation_graph
    {
    private:
        typedef std::vector< graph::edge_t >            edges_t;
        typedef typename Triangulation::Vertex_handle   vertex_t;
            
    public:
        typedef edges_t::const_iterator edges_iterator;

        triangulation_graph( Triangulation const & trg, std::map< vertex_t, size_t > & index )
                    : edges_( trg.number_of_vertices() )
        {
            typedef typename Triangulation::Finite_vertices_iterator    vertices_iterator;
            typedef typename Triangulation::Vertex_circulator           vertex_circulator;
            for ( vertices_iterator v = trg.vertices_begin(); v != trg.vertices_end(); ++v )
            {
                edges_t & edges = edges_[index[v]];
                vertex_circulator vc_begin = trg.incident_vertices( v ), vc( vc_begin );
                do 
                {
                    edges.push_back( graph::edge_t( index[vc], 1 ) );
                } while ( ++vc != vc_begin );
            }
        }

        size_t vertices_num() const
        {
            return edges_.size();
        }

        size_t degree( size_t v ) const
        {
            return edges_[v].size();
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
    };
}

#endif /* _TRIANGULATION_GRAPH_H_ */
