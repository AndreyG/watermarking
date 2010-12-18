#include "../stdafx.h"

#include "../geometry/planar_graph.h"
#include "stopwatch.h"
#include "debug_stream.h"

void add_intersection_points( geometry::planar_graph_t & graph )
{
    util::stopwatch _("fixing intersections in input graph");
	
	typedef geometry::planar_graph_t 									graph_t;
    typedef CGAL::Exact_predicates_inexact_constructions_kernel         GT;
    typedef CGAL::Exact_predicates_tag                                  Itag;
    typedef CGAL::Triangulation_vertex_base_2<GT>                       Vb;
    typedef CGAL::Constrained_triangulation_face_base_2<GT>             Fb;
    typedef CGAL::Triangulation_data_structure_2<Vb, Fb>                TDS;
    typedef CGAL::Constrained_Delaunay_triangulation_2<GT, TDS, Itag>   CDT;

    CDT trg;
    
	{
		util::stopwatch _("constrained triangulation construction");
	    for ( auto e = graph.edges_begin(); e != graph.edges_end(); ++e )
    	{
			graph_t::vertex_t const & v = graph.vertex(e->b);
			graph_t::vertex_t const & u = graph.vertex(e->e);
        	trg.insert_constraint(GT::Point_2(v.x(), v.y()), GT::Point_2(u.x(), u.y()));
    	}
	}
	if ( trg.number_of_vertices() == graph.vertices_num() )
	{
		return;
	}

	graph_t tmp;

    std::map< CDT::Vertex_handle, size_t > handle_to_index;
    for ( auto v = trg.finite_vertices_begin(); v != trg.finite_vertices_end(); ++v )
    {
        handle_to_index[v] = tmp.add_vertex(graph_t::vertex_t(v->point()));
    }

    for ( auto e = trg.finite_edges_begin(); e != trg.finite_edges_end(); ++e )
    {
		if ( !trg.is_constrained( *e ) )
			continue;

        auto face = e->first;
        int v = e->second;
		graph_t::edge_t edge;
        edge.b = handle_to_index[face->vertex(trg.cw(v))];
        edge.e = handle_to_index[face->vertex(trg.ccw(v))];
        tmp.add_edge( edge );
    }

	util::debug_stream() 	<< "input vertices num " << graph.vertices_num() 
							<< ", output verices num " << tmp.vertices_num();
	util::debug_stream() 	<< "input edges num " << graph.edges_num()
							<< ", output edges num " << tmp.edges_num();
	graph = tmp;
}

void fix_graph( geometry::planar_graph_t & g )
{
    add_intersection_points( g );

	typedef geometry::planar_graph_t 		graph_t;
    typedef graph_t::vertex_t 				vertex_t;
    typedef graph_t::edge_t 				edge_t;
    typedef std::map< vertex_t, size_t > 	v2i_t;  

	using geometry::point_t;

	point_t middle;
    for ( auto v = g.vertices_begin(); v != g.vertices_end(); ++v )
    {
		middle += *v;
    }
	middle.x() /= g.vertices_num();
	middle.y() /= g.vertices_num();
    for ( size_t v = 0; v != g.vertices_num(); ++v )
    {
        g.vertex( v ) -= middle;
    }

    v2i_t v2i;
    size_t idx = 0;
    for ( auto v = g.vertices_begin(); v != g.vertices_end(); ++v )
    {
        v2i_t::iterator it = v2i.find( *v );
        if ( it == v2i.end() )
        {    
            v2i.insert( it, std::make_pair( *v, idx ) );
            ++idx;
        }
    }
	graph_t tmp;

    std::set< vertex_t > vertex_set;
    for ( auto v = g.vertices_begin(); v != g.vertices_end(); ++v )
    {
        if ( !vertex_set.count( *v ) )
        {
            vertex_set.insert( *v );
            tmp.add_vertex( *v );
        }
    }

    for ( auto e = g.edges_begin(); e != g.edges_end(); ++e )
    {
		edge_t edge;
        edge.b = v2i[g.vertex(e->b)];
        edge.e = v2i[g.vertex(e->e)]; 
        if (edge.b != edge.e)
            tmp.add_edge( edge );
    }

	g = tmp;
}
