#ifndef _PLANE_SUBDIVISION_H_
#define _PLANE_SUBDIVISION_H_

#include "../algorithms/median.h"

namespace geometry
{
    template< class Point >
    struct point_comparator
    {
        point_comparator( bool horizontal )
                : horizontal_( horizontal )
        {}

        bool operator() ( Point const & left, Point const & right ) const
        {
            return horizontal_ ? left.x() < right.x() : left.y() < right.y();
        }

    private:
        bool horizontal_;
    };

    template< class Iter >
    size_t subdivide_plane( Iter p, Iter q, size_t max_subarea_size, bool horizontal, std::vector< size_t > & index, size_t existing_areas_num )
    {
        size_t size = std::distance( p, q );
        if ( size <= max_subarea_size )
        {
            for ( size_t i = 0; i != size; ++i )
                index.push_back( existing_areas_num );
            return existing_areas_num + 1;
        }
        else
        {
            typedef point_comparator< typename std::iterator_traits< Iter >::value_type > point_comparator;
            Iter m = algorithm::median( p, q, point_comparator( horizontal ) );
            existing_areas_num = subdivide_plane( p, m, max_subarea_size, !horizontal, index, existing_areas_num );
            return subdivide_plane( boost::next( m ), q, max_subarea_size, !horizontal, index, existing_areas_num );
        }
    }
}

#endif /* _PLANE_SUBDIVISION_ */
