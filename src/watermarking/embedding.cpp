#include "../stdafx.h"

#include "embedding.h"
#include "../geometry/plane_subdivision.h"
#include "../geometry/triangulation_graph.h"
#include "weights/unweighted.h"
#include "weights/conformal.h"
#include "weights/dirichlet.h"
#include "../inout/inout.h"
#include "../visualization/subdivided_plane_viewer.h"
#include "../visualization/triangulation_graph_viewer.h"

namespace watermarking
{
	namespace WeightType
	{
        Type from_str(std::string const & str)
        {
            if (str == "unweighted")
                return Unweighted;
			if (str == "conformal")
				return Conformal;
            if (str == "dirichlet")
                return Dirichlet;
            if (str == "sin-sum")
                return SinSum;
            if (str == "constrained-sin-sum")
                return ConstrainedSinSum;
            throw std::logic_error("There is no weight type with name = " + str);
        }

		const char * to_str( Type t )
		{
			static const char * name[TypeSize];
			name[Unweighted] = "unweighted";
			name[Conformal] = "conformal";
			name[Dirichlet] = "dirichlet";

			return name[t];
		}
	}

    embedding_impl::embedding_impl(    graph_t const & graph, size_t max_patch_size, 
                                       WeightType::Type weight_type, bool use_edges,
                                       bool step_by_step )
            : graph_( graph )
            , max_patch_size_( max_patch_size )
            , weight_type_( weight_type )
            , use_edges_( use_edges )
    {
        step_ = SUBDIVIDE_PLANE;
        if ( !step_by_step )
            while ( next_step() );
    }

    bool embedding_impl::next_step()
    {
        switch ( step_ )
        {
        case SUBDIVIDE_PLANE:
            subareas_num_ = subdivide_plane( max_patch_size_ );
            step_ = BUILD_TRIANGULATIONS;
            std::cout << "number of subareas " << subareas_num_ << std::endl;
            return true;
        case BUILD_TRIANGULATIONS:
            build_trgs( subareas_num_ );
            step_ = FACTORIZE;
            return true;
        case FACTORIZE:
            factorize( subareas_num_, weight_type_ );
            step_ = MODIFY_VERTICES;
            return true;
        default:
            return false;
        }
    }

    // vector subdivision_ will be filled by indices of zone in which graph_.vertices[i] will be laid.
    size_t embedding_impl::subdivide_plane( size_t max_subarea_size )
    {
        util::stopwatch _( "subidivide plane" );

        // ----------- types -------------
        typedef graph_t::vertex_t   vertex_t;
        typedef graph_t::edge_t     edge_t;
        // -------------------------------

        // --------- constants -----------
        const size_t N = graph_.vertices_num();
		const size_t M = graph_.edges_num();
        // -------------------------------
        
        std::map< vertex_t, size_t > old_index;
        size_t i = 0;
        for ( auto v = graph_.vertices_begin(); v != graph_.vertices_end(); ++v )
        {
            old_index.insert( std::make_pair( *v, i ) );
            ++i;
        }

		vertices_t tmp(graph_.vertices_begin(), graph_.vertices_end());
		
        size_t res = geometry::subdivide_plane( tmp.begin(), tmp.end(), max_subarea_size, true, subdivision_, 0 );
		for ( size_t i = 0; i != N; ++i )
		{
			graph_.vertex( i ) = tmp[i];
		}

        std::vector< size_t > old2new( N );
        for ( size_t i = 0; i != N; ++i )
        {
            old2new[old_index[graph_.vertex(i)]] = i;
        }

        for( size_t i = 0; i != M; ++i )
        {
			edge_t & edge = graph_.edge(i);
			edge.b = old2new[edge.b];
			edge.e = old2new[edge.e];
        }

        /*
        subdivided_plane_viewer_t viewer(&graph_, &subdivision_, res); 
        visualization::vis_system::run_viewer(&viewer);
        */

        return res;
    }

    void embedding_impl::modify_vertices( message_t const & message, size_t chip_rate, int key, double alpha )
    {
        util::stopwatch _("embedding message");

        step_ = STEP_SIZE;
        modified_vertices_.clear();

        for ( size_t s = 0, v = 0; s != subareas_num_; ++s )
        {
			size_t N = 1;
			while ((v + N != graph_.vertices_num()) && (subdivision_[v + N] == subdivision_[v]))
				++N;
            
			vertices_t old_vertices( graph_.vertices_begin() + v, graph_.vertices_begin() + v + N );
			v += N;

            size_t vectors_num = analysers_[s]->vectors_num(); 
            if (chip_rate * message.size() > vectors_num)
            {
                util::debug_stream(util::debug_stream::WARNING)
                    << "area: " << s 
                    << ": [wanted vectors num = " << chip_rate * message.size()
                    << ", calculated vectors num = " << vectors_num << "]";
                std::copy( old_vertices.begin(), old_vertices.end(), std::back_inserter( modified_vertices_ ) );
            }
            else
            {
                std::vector<double> r(vectors_num);

                srand(key);
                for ( size_t i = 0, k = 0; i != message.size(); ++i )
                {
                    for ( size_t j = 0; j != chip_rate; ++j, ++k )
                    {
                        int p = ( rand() % 2 ) * 2 - 1;
                        int b = message[i] * 2 - 1;
                        r[k] = b * p * alpha;
                    }
                }
                vertices_t new_vertices = analysers_[s]->get_vertices(r);
                for ( size_t i = 0; i != N; ++i )
                {
                    new_vertices[i] += old_vertices[i];
                }
                std::copy( new_vertices.begin(), new_vertices.end(), std::back_inserter( modified_vertices_ ) ); 
            }
        }
    }

	embedding_impl::Gt::Point_2 embedding_impl::toCGAL( vertex_t const & pt ) const
	{
		return Gt::Point_2( pt.x(), pt.y() );
	}

    void embedding_impl::build_trgs( size_t subareas_num )
    {
        util::stopwatch _( "build_triangulations" );
        trgs_.resize( subareas_num );

        for ( size_t i = 0, N = graph_.vertices_num(); i != N; ++i )
        {
            trgs_[subdivision_[i]].insert( toCGAL(graph_.vertex(i)) );
        }
        if ( use_edges_ )
        {
            util::stopwatch _( "insert constraints" );
			for ( auto edge = graph_.edges_begin(); edge != graph_.edges_end(); ++edge )
            {
                size_t begin = edge->b, end = edge->e;
                if ( subdivision_[begin] == subdivision_[end] )
                {
                    trg_t & trg = trgs_[subdivision_[begin]]; 
                    const trg_t::size_type old_vertices_num = trg.number_of_vertices();
                    trg.insert_constraint( toCGAL(graph_.vertex(begin)), toCGAL(graph_.vertex(end)) );
                    assert( trg.number_of_vertices() == old_vertices_num );
                }
            }
        }            
		// Я думаю, в этом нет необходиомости.
		// Этот код нужен для пущей уверенности в том, что порядок вершин в графе соответствует порядку вершин в триангуляции.
		std::vector< trg_t::Finite_vertices_iterator > vertices_iterator( subareas_num );
		for ( size_t i = 0; i != subareas_num; ++i )
		{
			vertices_iterator[i] = trgs_[i].finite_vertices_begin();
		}
		for ( size_t i = 0, N = graph_.vertices_num(); i != N; ++i )
		{
			graph_.vertex(i) = graph_t::vertex_t((vertices_iterator[subdivision_[i]]++)->point());
		}
		for ( size_t i = 0; i != subareas_num; ++i )
			assert( vertices_iterator[i] == trgs_[i].finite_vertices_end() );
    }

    void embedding_impl::factorize( size_t subareas_num, WeightType::Type type )
    {
        util::stopwatch _("coordinate vectors factorization");

        analysers_.resize( subareas_num );
        for ( size_t s = 0; s != subareas_num; ++s )
        {
            util::stopwatch _( boost::format("factorize coordinate vectors in subarea %d") % s );

            trg_t const &   trg         = trgs_[s];
            analyser_ptr &  analyser    = analysers_[s];

			geometry::triangulation_graph< trg_t > graph(trg);

            switch ( type )
			{
			case WeightType::Unweighted:
				analyser.reset( new unweighted_spectral_analyser(graph) );
				break;
			case WeightType::Conformal:
				analyser.reset( new conformal_spectral_analyser(graph) );
				break;
			case WeightType::Dirichlet:
				analyser.reset( new dirichlet_spectral_analyser(graph) );
				break;
			default:
				throw std::logic_error("unsupported type of analyser");
			}
        }
    }

    embedding_impl::embedding_impl( std::istream & in )
    {
        util::stopwatch _("embedding_impl: reading from file");

        in >> subareas_num_;
        analysers_.resize( subareas_num_ );
        foreach (analyser_ptr & analyser, analysers_ )
        {
			std::string type_str;
            do
            {
                in >> type_str;
                boost::trim(type_str);
            } while (type_str.empty() && in);
            weight_type_ = WeightType::from_str(type_str);

			switch ( weight_type_ )
			{
			case WeightType::Unweighted:
				analyser.reset( new unweighted_spectral_analyser( in ) );
				break;
			case WeightType::Conformal:
				analyser.reset( new conformal_spectral_analyser( in ) );
				break;
			case WeightType::Dirichlet:
				analyser.reset( new dirichlet_spectral_analyser( in ) );
				break;
			default:
				throw std::logic_error("unsupported type of analyser");
			}
        }
        inout::read_graph( graph_, in );
        subdivision_.resize( graph_.vertices_num( ) );
        foreach ( size_t & s, subdivision_ )
        {
            in >> s;
        }
    }

    void embedding_impl::dump( std::ostream & out ) const
    {
        out << subareas_num_ << std::endl;
        foreach (analyser_ptr const & analyser, analysers_ )
        {
			out << to_str(weight_type_) << "\n";
            analyser->dump(out);
        }
        inout::write_graph(graph_, out);
        foreach (size_t s, subdivision_ )
            out << s << " ";
    }
}
