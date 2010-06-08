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
            if ( comp( *m, value ) )
            {
                std::cout << "checker, value of bound = " << value << std::endl; 
                std::cout   << *prior( m )    << "\n" 
                            << *m             << "\n"  
                            << *next( m )     << std::endl;
                return false;
            }
            for ( ; p != m; ++p )
            {
                if ( !comp( *p, value ) )
                {
                    std::cout   << "checker, left part:\n" 
                                << *p << "\n" << value << std::endl;
                    return false;
                }
            }
            for ( ; m != q; ++m )
            {
                if ( comp( *m, value ) )
                {
                    std::cout   << "checker, right part:\n" 
                                << *m << "\n" << value << std::endl;
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
            typedef operator_binder< value_type, Comparator > predicate; 
            Iter bound = std::partition( p, q, predicate( middle, comp ) ); 
            assert( sequence_is_valid( p, q, bound, middle, comp ) );            
            return bound;
        }

        template< class Iter, class Comparator >
        Iter kth( Iter p, Iter q, size_t k, Comparator comp )
        {
            size_t dist = std::distance( p, q );
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
            if ( dist == k )
            {
                return m;
            }
            else if ( dist > k )
                return kth( p, m, k, comp );
            else
                return kth( m, q, k - dist, comp );
        }
    }

    template< class Iter, class Comparator >
    Iter median( Iter p, Iter q, Comparator comp )
    {
        Iter res = kth( p, q, std::distance( p, q ) / 2, comp );
        assert( ::abs( std::distance( p, res ) - std::distance( res, q ) ) <= 1 );
        typedef typename std::iterator_traits< Iter >::value_type value_type;
        for ( ; p != res; ++p )
        {
            if ( comp( *next( res ), *p ) )
            {
                std::cout << std::setprecision( 32 ) << *p << "\n" << *res << "\n" << *next( res ) << std::endl;
                assert( false );
            }
        }
        --q;
        for ( ; q != res; --q )
        {
            if ( comp( *q, *prior( res ) ) )
            {
                std::cout << std::setprecision( 32 ) << *res << "\n" << *q << std::endl;
                assert( false );
            }
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
