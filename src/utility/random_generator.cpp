#include "../stdafx.h"

#include <ctime>

#include "random_generator.h"

namespace util
{
    random_generator::random_generator(double radius)
        : rng_( time( NULL ) )
        , sampler_( rng_, normal_distribution( 0, radius ) )
    {}

    random_generator & random_generator::get(double radius)
    {
        cache_t::iterator it = cache_.find(radius);
        if (it != cache_.end())
            return *it->second;
        else
        {
            std::unique_ptr<random_generator> gen(new random_generator(radius));
            random_generator & res = *gen;
            cache_.insert(it, std::make_pair(radius, std::move(gen)));
            return res;
        }
    }

    random_generator::cache_t random_generator::cache_;
}
