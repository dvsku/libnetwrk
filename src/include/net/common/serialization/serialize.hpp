#ifndef LIBNETWRK_NET_COMMON_SERIALIZE_HPP
#define LIBNETWRK_NET_COMMON_SERIALIZE_HPP

#include <string>

#include "net/definitions.hpp"

namespace libnetwrk::net::common {
	// Serializes a primitive type
	template <typename T>
	static BUFFER_U8 serialize(const T& value) {
		static_assert(std::is_standard_layout<T>::value,
			"serialize() can only be called on standard data types. Implement serializable on your object.");

		BUFFER_U8 serialized;

		serialized.resize(sizeof(T));
		std::memcpy(serialized.data(), &value, sizeof(T));

		return serialized;
	}

	// Serializes a vector of primitive types
	template <typename T>
	static BUFFER_U8 serialize(const BUFFER_<T>& value) {
		static_assert(std::is_standard_layout<T>::value,
			"serialize() can only be called on standard data types. Implement serializable on your object.");

		BUFFER_U8 serialized;
		size_t total_size = value.size() * sizeof(T);
		size_t num_el = value.size();
		size_t offset = 0;

		// Elements + num of elements
		serialized.resize(total_size + sizeof(size_t));

		for (int i = value.size() - 1; i >= 0; i--) {
			BUFFER_U8 bytes = serialize(value[i]);
			std::memcpy(serialized.data() + offset, bytes.data(), bytes.size());
			offset += bytes.size();
		}

		// Num of elements
		std::memcpy(serialized.data() + offset, &num_el, sizeof(size_t));

		return serialized;
	}

	// Serializes a string
	static BUFFER_U8 serialize(const std::string& value) {
		BUFFER_U8 serialized;
		size_t string_size = value.length() + 1;

		serialized.resize(string_size + sizeof(size_t));

		std::memcpy(serialized.data(), value.c_str(), string_size);
		std::memcpy(serialized.data() + string_size, &string_size, sizeof(size_t));

		return serialized;
	}

	// Serializes a vector of strings
	static BUFFER_U8 serialize(const BUFFER_STR& value) {
		BUFFER_U8 serialized;
		size_t total_size = 0;
		size_t num_s = value.size();
		size_t offset = 0;

		// Length of each string + \0 + size int
		for (const std::string str : value) {
			total_size += str.length() + 1 + sizeof(size_t);
		}

		// Strings + num of strings
		serialized.resize(total_size + sizeof(size_t));

		for (int i = value.size() - 1; i >= 0; i--) {
			BUFFER_U8 bytes = serialize(value[i]);
			std::memcpy(serialized.data() + offset, bytes.data(), bytes.size());
			offset += bytes.size();
		}

		// Num of strings
		std::memcpy(serialized.data() + offset, &num_s, sizeof(size_t));

		return serialized;
	}
}

#endif