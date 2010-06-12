#ifndef _ALGORITHM_UTIL_H_
#define _ALGORITHM_UTIL_H_

namespace algorithm
{
	template< typename T >
	bool make_min( T & a, T const & b )
	{
		if ( b < a )
		{
			a = b;
			return true;
		}
		return false;
	}

	template< typename T >
	bool make_max( T & a, T const & b )
	{
		if ( b > a )
		{
			a = b;
			return true;
		}
		return false;
	}

    template< class Iter >
    Iter find_last( Iter begin, Iter end, typename Iter::value_type const & v )
    {
        typedef Iter iter_t;
        typedef typename std::reverse_iterator< iter_t > r_iter_t;
        r_iter_t ri = std::find( r_iter_t( end ), r_iter_t( begin ), v );
        iter_t i = ri.base();
        if ( i == begin )
            return end;
        else
            return --i;
    }
}

#endif /* _ALGORITHM_UTIL_H_ */
