#ifndef _TRIGONOMETRY_H_
#define _TRIGONOMETRY_H_

namespace geometry
{
    double ctg( point_t p1, point_t const & q, point_t p2 )
    {
        p1 -= q;
        p2 -= q;

    	double c = p1 * p2;
    	double s = abs(p1 ^ p2);
        if ( s == 0 )
        {
        	if ( c > 0 )
        		return std::numeric_limits< double >::max();
        	else
        		return -std::numeric_limits< double >::max();
        }
        return c / s;
    }
}

#endif
