#ifndef LIBNETWRK_UTILITIES_BUFFERS_H
#define LIBNETWRK_UTILITIES_BUFFERS_H

#include "net/definitions.hpp"

namespace libnetwrk::utilities {
	static void insert_bytes(BUFFER_U8& dst, const BUFFER_U8& src);
	
	static BUFFER_U8 get_range(BUFFER_U8& src, size_t start, size_t count);
}

#endif