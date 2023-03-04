#ifndef LIBNETWRK_UTILITIES_BUFFERS_HPP
#define LIBNETWRK_UTILITIES_BUFFERS_HPP

#include "libnetwrk/net/definitions.hpp"

namespace libnetwrk::utilities {
	static void push_bytes(BUFFER_U8& dst, const BUFFER_U8& src) {
		size_t dst_old_size = dst.size();
		dst.resize(dst.size() + src.size());
		std::memcpy(&dst[0] + dst_old_size, &src[0], src.size());
	}
	
	static BUFFER_U8 get_range(BUFFER_U8& src, size_t start, size_t count) {
		BUFFER_U8 result;
		result.resize(start + count);
		std::memcpy(&result[0], &src[0] + start, count);
		return result;
	}
}

#endif