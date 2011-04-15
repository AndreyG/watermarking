#ifndef _MEDIAN_H_
#define _MEDIAN_H_

#include <algorithm>
#include <boost/utility.hpp>

#include "../utility/stopwatch.h"
#include "../utility/debug_stream.h"

namespace algorithm
{
    namespace 
    {
        template< class Iter >
        struct default_comparator_f
        {
            typedef std::less< typename std::iterator_traits< Iter >::value_type > type;
        };

        template< class Iter, class Comparator, class ValueType >
        bool sequence_is_valid( Iter p, Iter q, Iter m, ValueType const & value, Comparator comp )
        {
            if ( comp( *m, value ) )
            {
                return false;
            }
            for ( ; p != m; ++p )
            {
                if ( !comp( *p, value ) )
                {
                    return false;
                }
            }
            for ( ; m != q; ++m )
            {
                if ( comp( *m, value ) )
                {
                    return false;
                }
            }
            return true;
        }

        template< class T, class Comparator >
        struct operator_binder
        {
            operator_binder( T const & value, Comparator const & comp )
                    : value_( value )
                    , comp_( comp )
            {}
  
            bool operator() ( T const & t ) const
            {
                return comp_( t, value_ );
            }
        private :
            T           const & value_;
            Comparator  const & comp_;
        };

        template< class Iter, class Comparator >
        Iter part( Iter p, Iter q, Comparator const & comp )
        {
            typedef typename std::iterator_traits< Iter >::value_type value_type;
            const typename std::iterator_traits< Iter >::difference_type size = std::distance( p, q );
            value_type middle = *( p + (size / 2) );
            typedef operator_binder< value_type, Comparator > predicate; 

            Iter bound = std::partition( p, q, predicate( middle, comp ) ); 
            std::swap( *bound, *std::find( bound, q, middle ) );
            
            assert( sequence_is_valid( p, q, bound, middle, comp ) );
            return bound;
        }

        template< class Iter, class Comparator >
        Iter kth( Iter p, Iter q, typename std::iterator_traits< Iter >::difference_type k, Comparator const & comp )
        {
            typename std::iterator_traits< Iter >::difference_type dist = std::distance( p, q );
            assert(k <= dist);

            if ( dist == 1 )
                return p;
            if ( dist == 2 )
            {
                if ( comp( *next( p ), *p ) )
                    std::swap( *p, *next( p ) );
                return p + k;
            }

            Iter m = part( p, q, comp );
            dist = std::distance( p, m );
            
            if ( k < dist )
                return kth( p, m, k, comp );
            else if ( k == dist ) 
                return m;
            else
                return kth( m + 1, q, k - (dist + 1), comp );
        }
    }

    template< class Iter, class Comparator >
    Iter median( Iter p, Iter q, Comparator comp )
    {
        Iter res = kth( p, q, std::distance( p, q ) / 2, comp );
        assert( ::abs( std::distance( p, res ) - std::distance( res, q ) ) <= 1 );
        for ( ; p != res; ++p )
        {
            assert( !comp( *res, *p ) );
        }
        --q;
        for ( ; q != res; --q )
        {
            assert( !comp( *q, *res ) );
        }
        return res;
    }

    template< class Iter >
    Iter median( Iter p, Iter q )
    {
        typedef typename default_comparator_f< Iter >::type comp_t; 
        return median( p, q, comp_t() );
    }
}

#endif /* _MEDIAN_H */
