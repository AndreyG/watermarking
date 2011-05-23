#ifndef _EMBEDDING_H_
#define _EMBEDDING_H_

/**
 * \file watermarking/embedding.h
 */

#include "common.h"

using boost::lexical_cast;

namespace watermarking
{
    namespace WeightType
    {
        //! Spectral analysers types
        /*!
         * To see supporting now types go to \link from_str(std::string const &) definition.
         */
        enum Type
        {
            Unweighted, Conformal, Dirichlet, SinSum, ConstrainedSinSum, TypeSize
        };

        Type from_str(std::string const & str);
    }
    
    /*! 
     * \brief Main class for embedding watermarking 
     */
    struct embedding_impl
    {
        typedef geometry::planar_graph_t  								graph_t;
        typedef CGAL::Exact_predicates_inexact_constructions_kernel     Gt;
        typedef CGAL::Constrained_Delaunay_triangulation_2< Gt >        trg_t;

		typedef graph_t::vertex_t										vertex_t;
        typedef std::vector< vertex_t >                                 vertices_t;

        enum step_t
        {
            SUBDIVIDE_PLANE, BUILD_TRIANGULATIONS, FACTORIZE, MODIFY_VERTICES, STEP_SIZE
        };
        
        //! Main constructor
        /*!
         * \param graph input graph
         * \param max_patch_size maximal number of vertices in one subarea
         * \param type type of spectral analyser used for embedding
         * \param use_edges defines if constrained or not triangulation will be used to constructing graph for spectral anaylysis
         * \param step_by_step defines if following steps: {subdivide plane, build triangulations, factorize triangulation graphs}
         *                              will be maked simultaneously
         */
        embedding_impl( graph_t const & graph, size_t max_patch_size, 
                        WeightType::Type type, bool use_edges, bool step_by_step );

        void modify_vertices( message_t const & message, size_t chip_rate, int key, double alpha );

        //! Reading constructor
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

        graph_t const & rearranged_graph() const { return graph_; }

        std::vector< analyser_ptr > const & get_analysers() const { return analysers_; }

        std::vector< size_t > const & get_subdivision() const { return subdivision_; }

    private:

        size_t subdivide_plane( size_t max_subarea_size );

        void build_trgs( size_t subareas_num );

        void factorize( size_t subareas_num, WeightType::Type );
 
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
