#ifndef _STOPWATCH_H_
#define _STOPWATCH_H_

#include <ctime>

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
            start = clock();
        }

        ~stopwatch()
        {
            --tabs_num;
            for ( size_t i = 0; i != tabs_num; ++i )
                std::cout << "\t";
            clock_t end = clock();
            std::cout << double(end - start) / CLOCKS_PER_SEC << " sec" << std::endl;
        }

    private:
        clock_t start;
    };

    size_t stopwatch::tabs_num = 0;
}
#endif /* _STOPWATCH_H_ */
