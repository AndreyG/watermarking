#ifndef _POINT_H_
#define _POINT_H_

namespace geometry
{
    struct point_t
    {
		point_t()
			: x_( 0 )
			, y_( 0 )
		{}

		point_t( double x, double y )
			: x_( x )
			, y_( y )
		{}

		point_t( point_t const & pt )
			: x_( pt.x() )
			, y_( pt.y() )
		{}

        template< class Point >
        explicit point_t( Point const & pt )
            : x_( pt.x() )
            , y_( pt.y() )
        {}

#define OPERATOR_APPLY_POINT(op) point_t & operator op##= (point_t const & pt ) \
    { \
        x() op##= pt.x(); \
        y() op##= pt.y(); \
        return *this; \
    }

        OPERATOR_APPLY_POINT(+)
        OPERATOR_APPLY_POINT(-)

#define OPERATOR_APPLY_NUMBER(op) point_t & operator op##= (double d) \
    { \
        x() op##= d; \
        y() op##= d; \
        return *this; \
    }

        OPERATOR_APPLY_NUMBER(*)
        OPERATOR_APPLY_NUMBER(/)

        double & x() { return x_; }
        double & y() { return y_; }

        double const & x() const { return x_; }
        double const & y() const { return y_; }

    private:
        double x_, y_;
    };

	inline bool operator == (point_t const & a, point_t const & b)
	{
		return (a.x() == b.x()) && (a.y() == b.y());
	}

	inline bool operator != (point_t const & a, point_t const & b)
	{
		return !(a == b);
	}

	inline bool operator < (point_t const & a, point_t const & b)
	{
		if (a.x() == b.x())
			return a.y() < b.y();
		else
			return a.x() < b.x();
	}

    inline point_t operator + (point_t const & a, point_t const & b)
    {
        point_t tmp(a);
        tmp += b;
        return tmp;
    }

    inline point_t operator - (point_t const & a, point_t const & b)
    {
        point_t tmp(a);
        tmp -= b;
        return tmp;
    }

    inline point_t operator * (point_t const & p, double d)
    {
        point_t tmp(p);
        tmp *= d;
        return tmp;
    }

    inline point_t operator / (point_t const & p, double d)
    {
        point_t tmp(p);
        tmp /= d;
        return tmp;
    }

    inline double operator * (point_t const & a, point_t const & b)
    {
        return a.x() * b.x() + a.y() * b.y();
    }

    inline double operator ^ (point_t const & a, point_t const & b)
    {
        return a.x() * b.y() - a.y() * b.x();
    }

	template<class Stream>
	Stream& operator << (Stream & out, point_t const & pt)
	{
    	out << "(" << pt.x() << ", " << pt.y() << ")";
    	return out;
	}

	inline double sqr( double x ) { return x * x; }

    inline double mod( point_t const & v )
    {
        return sqr( v.x() ) + sqr( v.y() );
    }

	inline point_t from_polar( double r, double phi )
	{
		return point_t(r * cos(phi), r * sin(phi));
	}
}

#endif /*_POINT_H_*/
