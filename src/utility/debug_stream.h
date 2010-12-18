#ifndef _DEBUG_STREAM_H_
#define _DEBUG_STREAM_H_

namespace util
{
	struct debug_stream
	{
		static void inc_indent();
		static void dec_indent();
		static bool is_first();

		template<class Data>
		debug_stream& operator << (Data const & data)
		{
			ss_ << data;
			return *this;
		}

		~debug_stream();

	private:
		std::stringstream ss_;
        static size_t tabs_num;
		static std::stack< bool > is_first_;
	};
}

#endif /*_DEBUG_STREAM_*/
