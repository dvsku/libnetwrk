#ifndef LIBNETWRK_NET_COMMON_SERIALIZABLE_HPP
#define LIBNETWRK_NET_COMMON_SERIALIZABLE_HPP

#include "libnetwrk/net/definitions.hpp"

namespace libnetwrk::net::common {
	// Base struct for every user made serializable object
	struct serializable {
		// Serializes struct to vector of bytes
		virtual BUFFER_U8 serialize() const = 0;

		// Deserializes a struct from a vector of bytes
		virtual void deserialize(BUFFER_U8 serialized) = 0;

		// Returns the size of the struct
		virtual size_t size() const = 0;
	};
}

#endif