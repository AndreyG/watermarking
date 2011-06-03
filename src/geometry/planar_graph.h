#ifndef _PLANAR_GRAPH_
#define _PLANAR_GRAPH_

#include "../utility/random_generator.h"
#include "point.h"

namespace geometry
{
    struct planar_graph_t
    {
        // -------------------- typedefs ------------------- //
        typedef point_t                     				vertex_t;
		typedef std::vector< vertex_t >::const_iterator 	vertices_iterator;

		struct edge_t
		{
			size_t b, e;
		};

		typedef std::vector< edge_t >::const_iterator 		edges_iterator;

        // ---------------- constant accessors --------------- //
		size_t vertices_num()   const { return vertices_.size();    }
		size_t edges_num()      const { return edges_.size();       }

		vertex_t const & vertex( size_t v ) const { return vertices_[v];        }

		vertices_iterator vertices_begin()  const { return vertices_.begin();   }
		vertices_iterator vertices_end()    const { return vertices_.end();     }
		
		edge_t const & edge( size_t e ) const { return edges_[e];       }

		edges_iterator edges_begin()    const { return edges_.begin();  }
		edges_iterator edges_end()      const { return edges_.end();    }
		
        // ------------------ mutators ---------------------- //
		vertex_t    & vertex( size_t v ) { return vertices_[v]; }
		edge_t      & edge  ( size_t e ) { return edges_[e];    }

		size_t add_vertex( vertex_t const & v )
		{
			vertices_.push_back( v );
			return vertices_num() - 1;
		}

		size_t add_edge( edge_t const & e )
		{
			edges_.push_back( e );
			return edges_num() - 1;
		}

		template<class FwdIter>
		void add_vertices(FwdIter p, FwdIter q)
		{
			vertices_.insert(vertices_.end(), p, q);
		}

		template<class FwdIter>
		void add_edges(FwdIter p, FwdIter q)
		{
			edges_.insert(edges_.end(), p, q);
		}

	private:
		typedef std::vector< vertex_t >		vertices_t;
        typedef std::vector< edge_t >       edges_t;

        vertices_t   vertices_;
        edges_t      edges_;
    };

    inline planar_graph_t add_noise( planar_graph_t const & g, double radius )
    {
        util::random_generator gen( radius );

		planar_graph_t res;
		res.add_edges(g.edges_begin(), g.edges_end());
		for ( auto v = g.vertices_begin(); v != g.vertices_end(); ++v )
        {
            double a = ( 2 * M_PI * rand() ) / RAND_MAX;
            double r = gen();
        	res.add_vertex( *v + from_polar(r, a) );  
        }
        return res;
    }
}

#endif /*_PLANAR_GRAPH_*/
