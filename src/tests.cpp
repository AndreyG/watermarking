#include "stdafx.h"
#include "algorithms/median.h"

int main(int argc, char **argv)
{
    std::vector<int> v(10);
    for (size_t i = 0; i != v.size(); ++i)
        v[i] = rand() % 100;
    std::copy(v.begin(), v.end(), std::ostream_iterator<int>(std::cout, " "));
    std::cout << std::endl;
    std::cout << *algorithm::median(v.begin(), v.end()) << std::endl;
    std::copy(v.begin(), v.end(), std::ostream_iterator<int>(std::cout, " "));
    std::cout << std::endl;
}
