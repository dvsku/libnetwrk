#include "utilities/buffers.hpp"

void libnetwrk::utilities::insert_bytes(BUFFER_U8& dst, const BUFFER_U8& src) {
	size_t dst_old_size = dst.size();
	dst.resize(dst.size() + src.size());
	std::memcpy(&dst[0] + dst_old_size, &src[0], src.size());
}

BUFFER_U8 libnetwrk::utilities::get_range(BUFFER_U8& src, size_t start, size_t count) {
	BUFFER_U8 result;
	result.resize(start + count);
	std::memcpy(&result[0], &src[0] + start, count);
	return result;
}