#ifndef _EMBEDDING_H_
#define _EMBEDDING_H_

namespace watermarking
{
    const size_t MAX_PATCH_SIZE = 1000;

    template<class Point>
    struct embedding_impl
    {
        typedef Point                       vertex_t;
        typedef std::vector< vertex_t >     vertices_t;
        typedef std::pair< size_t, size_t > edge_t;
        typedef std::vector< edge_t >       edges_t;

        empbedding_impl( vertices_t const & vertices, edges_t const & edges )
        {
            std::vector< size_t > index( vertices.size() );
            size_t patches_num = subdivide_plane( vertices, MAX_PATCH_SIZE, index );
        }
    }
}

#endif /* _EMBEDDING_H_ */
