#ifndef _RANDOM_GENERATOR_H_
#define _RANDOM_GENERATOR_H_

#include <boost/random.hpp>
#include <ctime>

namespace util
{
    using boost::mt19937;
    using boost::variate_generator;

    struct random_generator
    {
        random_generator( double radius )
            : rng_( time( NULL ) )
            , sampler_( rng_, normal_distribution( 0, radius ) )
        {}

        double operator() ()
        {
            return sampler_();
        }

    private:        
        typedef boost::normal_distribution< double > normal_distribution;

        mt19937 rng_;
        variate_generator< mt19937&, normal_distribution > sampler_;
    };
}

#endif /* _RANDOM_GENERATOR_H_ */
