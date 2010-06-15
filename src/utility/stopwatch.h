#ifndef _STOPWATCH_H_
#define _STOPWATCH_H_

#include <time.h>

namespace util
{
    struct stopwatch
    {
    private:
        static size_t tabs_num;

    public:
        explicit stopwatch( const char * message )
        {
            for ( size_t i = 0; i != tabs_num; ++i )
                std::cout << "\t";
            std::cout << message << "..." << std::endl;
            ++tabs_num;
            time( &start );
        }

        ~stopwatch()
        {
            --tabs_num;
            for ( size_t i = 0; i != tabs_num; ++i )
                std::cout << "\t";
            time_t end;
            time( &end );
            std::cout << difftime( end, start ) << " sec" << std::endl;
        }

    private:
        time_t start;
    };

    size_t stopwatch::tabs_num = 0;
}
#endif /* _STOPWATCH_H_ */
