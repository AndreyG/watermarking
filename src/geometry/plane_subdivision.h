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
            if ( horizontal_ )
            {
                if ( left.x() == right.x() )
                    return left.y() < right.y();
                else
                    return left.x() < right.x();
            }
            else
            {
                if ( left.y() == right.y() )
                    return left.x() < right.x();
                else
                    return left.y() < right.y();
            }
        }

    private:
        bool horizontal_;
    };

    // [p, q) -- sequence of points
    // max_subarea_size -- maximal number of points in one subarea
    // horizontal -- horizontal or vertical line will be used to sibdivide at this step
    // index -- is filled by indices of zone in which point will be laid.
    //
    // returns number of new subareas
    template< class Iter >
    size_t subdivide_plane( Iter p, Iter q, size_t max_subarea_size, bool horizontal,
							std::vector< size_t > & index, size_t existing_areas_num )
    {
        size_t size = std::distance( p, q );
        if ( size <= max_subarea_size )
        {
            for ( size_t i = 0; i != size; ++i )
                index.push_back( existing_areas_num );
            return 1;
        }
        else
        {
            typedef typename std::iterator_traits< Iter >::value_type point_t;            
            typedef point_comparator< point_t > point_comparator;
            
            point_comparator comp( horizontal ); 
            Iter m = algorithm::median( p, q, comp );
            size_t tmp = subdivide_plane( 	p, m, max_subarea_size, !horizontal, index,
											existing_areas_num );
            return tmp + subdivide_plane( 	m, q, max_subarea_size, !horizontal, index,
											existing_areas_num + tmp );
        }
    }
}

#endif /* _PLANE_SUBDIVISION_ */
