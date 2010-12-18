#ifndef _STOPWATCH_H_
#define _STOPWATCH_H_

#include <ctime>

namespace util
{
    struct stopwatch
    {
    public:
        explicit stopwatch( const char * message );
        ~stopwatch();

    private:
        clock_t start;
    };
}
#endif /* _STOPWATCH_H_ */
