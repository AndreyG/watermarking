#include <vector>
#include <fstream>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <boost/utility.hpp>
#include <assert.h>
#include <boost/bind.hpp>

#include "utility/dump.h"
#include "algorithms/median.h"

int main( int argc, char** argv )
{
    std::vector< int > vec;
    std::ifstream input( "input/test.txt" );
    std::istream_iterator< int > begin( input ), end;
    std::copy( begin, end, std::back_inserter( vec ) );
    std::vector< int >::iterator median = algorithm::median( vec.begin(), vec.end() );
    std::cout << *median << std::endl;
    util::dump_sequence( vec.begin(), vec.end() ); 
}
