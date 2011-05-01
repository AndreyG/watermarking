#ifndef _RECTANGLE_H_
#define _RECTANGLE_H_

#include "../algorithms/util.h"

namespace geometry
{
    static const double POS_INF = std::numeric_limits<double>::max();
    static const double NEG_INF = -POS_INF;

struct rectangle_t
{
    rectangle_t()
        : left_bottom_  (POS_INF, POS_INF)
        , right_top_    (NEG_INF, NEG_INF)
    {}

    rectangle_t & operator &= (point_t const & pt)
    {
        using algorithm::make_min;
        using algorithm::make_max;

        make_min(left_bottom_.x(), pt.x());
        make_min(left_bottom_.y(), pt.y());
        make_max(right_top_.x(), pt.x());
        make_max(right_top_.y(), pt.y());

        return *this;
    }

    point_t xy() const { return left_bottom_; }
    point_t xY() const { return point_t(left_bottom_.x(), right_top_.y()); }
    point_t Xy() const { return point_t(right_top_.x(), left_bottom_.y()); }
    point_t XY() const { return right_top_; }

private:
    point_t left_bottom_;
    point_t right_top_;
};

}
#endif
