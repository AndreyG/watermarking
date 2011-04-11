#ifndef _STOPWATCH_H_
#define _STOPWATCH_H_

#include <ctime>

namespace util
{
    struct stopwatch
    {
    public:
        explicit stopwatch( const char * message );
        explicit stopwatch( std::string const & message );
        explicit stopwatch( boost::format const & fmt );
        ~stopwatch();

    private:
        void init( const char * message );
        clock_t start;
    };
}
#endif /* _STOPWATCH_H_ */
