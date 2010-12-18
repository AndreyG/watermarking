#ifndef _EMBEDDING_H_
#define _EMBEDDING_H_

#include "../spectral_analysis.h"
#include "../geometry/planar_graph.h"

using boost::lexical_cast;

namespace watermarking
{
    namespace WeightType
    {
        enum Type
        {
            Unweighted, Ctg, SinSum, ConstrainedSinSum
        };

        Type from_str(std::string const & str);
    }
    
	typedef std::vector< int > message_t;

    struct embedding_impl
    {
        typedef geometry::planar_graph_t  								graph_t;
        typedef CGAL::Exact_predicates_inexact_constructions_kernel     Gt;
        typedef CGAL::Constrained_Delaunay_triangulation_2< Gt >        trg_t;

		typedef graph_t::vertex_t										vertex_t;
        typedef std::vector< vertex_t >                                 vertices_t;
		typedef boost::shared_ptr< spectral_analyser >					analyser_ptr;

        enum step_t
        {
            SUBDIVIDE_PLANE, BUILD_TRIANGULATIONS, FACTORIZE, MODIFY_VERTICES, STEP_SIZE
        };
        
        embedding_impl( graph_t const & graph, size_t max_patch_size, 
                        WeightType::Type, bool use_edges, bool step_by_step );

        explicit embedding_impl( std::istream & in );

        void dump( std::ostream & out ) const;
        
        bool next_step();

        graph_t modified_graph() const
        {
            graph_t res;
            res.add_vertices(modified_vertices_.begin(), modified_vertices_.end());
            res.add_edges(graph_.edges_begin(), graph_.edges_end());
            return res;
        }

        graph_t const & rearranged_graph() const
        {
            return graph_;
        }

        std::vector< analyser_ptr > const & get_analysers() const
        {
            return analysers_;
        }

        std::vector< size_t > const & get_subdivision() const
        {
            return subdivision_;
        }

//    private:

        size_t subdivide_plane( size_t max_subarea_size );

        void build_trgs( size_t subareas_num );

        void factorize( size_t subareas_num, WeightType::Type );

        void modify_vertices( message_t const & message, size_t chip_rate, int key, double alpha );
 
 	private:
		Gt::Point_2 toCGAL( vertex_t const & pt ) const;

        size_t                              subareas_num_;
        graph_t                             graph_;
        std::vector< trg_t >                trgs_;
        std::vector< size_t >               subdivision_;
        vertices_t                          modified_vertices_;
        std::vector< analyser_ptr >         analysers_;
        step_t                              step_;

        size_t                              max_patch_size_;
        WeightType::Type          			weight_type_;
        bool                                use_edges_;
    };
}

#endif /* _EMBEDDING_H_ */
