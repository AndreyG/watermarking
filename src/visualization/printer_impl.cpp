#include "../stdafx.h"

#include "printer_impl.h"

namespace visualization
{

struct stream_impl : stream_t
{
    explicit stream_impl(boost::function<void (const char *)> const & write)    
        : write_(write)
    {}

#define PRINT(type) \
    stream_t & operator << (type t) \
    { \
        ss_ << t; \
        return *this; \
    }
    PRINT(const char *)
    PRINT(std::string const &)
    PRINT(size_t)
    PRINT(std::complex<double> const &)
    PRINT(point_t const &)
#undef PRINT

    ~stream_impl() { write_(ss_.str().c_str()); }

private:
    std::stringstream ss_;
    boost::function<void (const char*)> write_; 
};

std::unique_ptr<stream_t> printer_impl::corner_stream()
{
    std::unique_ptr<stream_t> stream(new stream_impl(boost::bind(draw_string_corner_, 10, corner_stream_height_indent_, _1)));
    corner_stream_height_indent_ += 15;
    return stream;
}

std::unique_ptr<stream_t> printer_impl::global_stream(point_t const & pt)
{
    return std::unique_ptr<stream_t>(new stream_impl(boost::bind(draw_string_global_, pt.x(), pt.y(), _1)));
}

}
