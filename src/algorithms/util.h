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
}
#endif /* _ALGORITHM_UTIL_H_ */
