#ifndef _MEDIAN_H_
#define _MEDIAN_H_

#include <algorithm>

namespace algorithm
{
    namespace 
    {
        using boost::next;
        using boost::prior;

        template< class Iter >
        struct default_comparator_f
        {
            typedef std::less< typename std::iterator_traits< Iter >::value_type > type;
        };

        template< class Iter, class Comparator, class ValueType >
        bool sequence_is_valid( Iter p, Iter q, Iter m, ValueType const & value, Comparator comp )
        {
            for ( ; p != m; ++p )
            {
                if ( !comp( *p, value ) )
                {
                    std::cout   << "checker: " 
                                << *p << "\t" << value << std::endl;

                    return false;
                }
            }
            --q;
            for ( ; q != m; --q )
            {
                if ( comp( *q, value ) )
                {
                    std::cout   << "checker, right part: " 
                                << *q << "\t" << value << std::endl;
                    return false;
                }
            }
            return true;
        }

        template< class T, class Comparator >
        struct operator_binder
        {
            operator_binder( T const & value, Comparator & comp )
                    : value_( value )
                    , comp_( comp )
            {}
  
            bool operator() ( T const & t ) const
            {
                return comp_( t, value_ );
            }
        private :
            T const & value_;
            Comparator & comp_;
        };

        template< class Iter, class Comparator >
        Iter part( Iter p, Iter q, Comparator comp )
        {
            typedef typename std::iterator_traits< Iter >::value_type value_type;
            int size = std::distance( p, q );
            value_type middle = *( p + rand() % size );
            //dump_sequence( p, q );
            //std::cout << "middle: " << middle << std::endl;
            typedef operator_binder< value_type, Comparator > predicate; 
            Iter m = std::partition( p, q, predicate( middle, comp ) ); 
            //std::cout << "bound: " << *m << std::endl;
            //dump_sequence( p, q );
            assert( sequence_is_valid( p, q, m, middle, comp ) );
            return m;
        }

        template< class Iter, class Comparator >
        Iter kth( Iter p, Iter q, size_t k, Comparator comp )
        {
            Iter m = part( p, q, comp );
            size_t dist = std::distance( p, m );
            if ( dist == k )
                return m;
            else if ( dist > k )
                return kth( p, m, k, comp );
            else 
                return kth( m, q, k - dist, comp );
        }
    }

    template< class Iter, class Comparator >
    Iter median( Iter p, Iter q, Comparator comp )
    {
        return kth( p, q, std::distance( p, q) / 2, comp );
    }

    template< class Iter >
    Iter median( Iter p, Iter q )
    {
        typedef typename default_comparator_f< Iter >::type comp_t; 
        return median( p, q, comp_t() );
    }
}

#endif /* _MEDIAN_H */
