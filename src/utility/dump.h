#ifndef _UTILITY_DUMP_H_
#define _UTILITY_DUMP_H_

namespace util
{
    template< class Iter, class Stream >
    void dump_sequence( Iter p, Iter q, Stream& stream )
    {
        typedef typename std::iterator_traits< Iter >::value_type value_type;
        std::copy( p, q, std::ostream_iterator< value_type >( stream, " " ) );
        stream << std::endl;
    }

    template< class Iter >
    void dump_sequence( Iter p, Iter q )
    {
        dump_sequence( p, q, std::cout );
    }
}

#endif /* _UTILITY_DUMP_H_ */
