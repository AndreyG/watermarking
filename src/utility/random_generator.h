#ifndef _RANDOM_GENERATOR_H_
#define _RANDOM_GENERATOR_H_

#include <boost/random.hpp>

namespace util
{
    using boost::mt19937;
    using boost::variate_generator;

    struct random_generator
    {
        double operator() ()
        {
            return sampler_();
        }

        static random_generator & get(double radius);

    private:        
        typedef boost::normal_distribution< double > normal_distribution;
        typedef std::map<double, std::unique_ptr<random_generator>> cache_t;
        static cache_t cache_;

        random_generator(double radius);

        mt19937 rng_;
        variate_generator< mt19937&, normal_distribution > sampler_;
    };
}

#endif /* _RANDOM_GENERATOR_H_ */
