#ifndef _INOUT_UTIL_H_
#define _INOUT_UTIL_H_

namespace inout
{
	template< class Stream >
	void skip_char( Stream & in, char barrier )
	{
		while ( in )
		{
			char c;
			in >> c;
			if ( c == barrier )
				break;
		}
	}
}

#endif /*_INOUT_UTIL_H_*/
