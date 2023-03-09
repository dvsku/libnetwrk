#ifndef LIBNETWRK_NET_COMMON_BINARY_SERIALIZER_HPP
#define LIBNETWRK_NET_COMMON_BINARY_SERIALIZER_HPP

#include "libnetwrk/net/common/containers/buffer.hpp"
#include "libnetwrk/net/common/serialization/type_traits.hpp"

namespace libnetwrk::net::common {
	struct binary_serializer {
		typedef buffer<binary_serializer> buffer_t;

		///////////////////////////////////////////////////////////////////////
		// Standard layout
		///////////////////////////////////////////////////////////////////////

		template <typename T>
		static void serialize(buffer_t& buffer, const T& value,
			typename std::enable_if<std::is_standard_layout<T>::value, bool>::type = true) 
		{
			buffer.push_back(&value, sizeof(T));
		}

		template <typename T>
		static void deserialize(buffer_t& buffer, T& obj,
			typename std::enable_if<std::is_standard_layout<T>::value, bool>::type = true)
		{
			std::memcpy(&obj, buffer.data() + buffer.offset(), sizeof(obj));
			buffer.advance(sizeof(obj));
		}

		///////////////////////////////////////////////////////////////////////
		// Serializable
		///////////////////////////////////////////////////////////////////////

		template <typename T>
		static void serialize(buffer_t& buffer, const T& obj,
			typename std::enable_if<is_serializable<T, binary_serializer>::value, bool>::type = true) 
		{
			buffer_t serialized = obj.serialize();
			buffer.push_back(serialized.size());
			buffer.push_back(serialized);
		}

		template <typename T>
		static void deserialize(buffer_t& buffer, T& obj,
			typename std::enable_if<is_serializable<T, binary_serializer>::value, bool>::type = true)
		{
			size_t size = 0;
			buffer >> size;
			obj.deserialize(buffer.get_range(buffer.offset(), size));
		}

		///////////////////////////////////////////////////////////////////////
		// std::string
		///////////////////////////////////////////////////////////////////////

		static void serialize(buffer_t& buffer, const std::string& str) {
			serialize(buffer, str.size());
			for (const char& element : str)
				serialize(buffer, element);
		}

		static void deserialize(buffer_t& buffer, std::string& str) {
			size_t size = 0;

			// Get # of elements
			buffer >> size;

			for (size_t i = 0; i < size; i++) {
				char element = 0;
				deserialize(buffer, element);
				str.push_back(element);
			}
		}

		///////////////////////////////////////////////////////////////////////
		// std::vector
		///////////////////////////////////////////////////////////////////////

		template <typename T>
		static void serialize(buffer_t& buffer, const std::vector<T>& container) {
			serialize(buffer, container.size());
			for (const T& element : container)
				serialize(buffer, element);
		}

		template <typename T>
		static void deserialize(buffer_t& buffer, std::vector<T>& container) {
			size_t size = 0;

			// Get # of elements
			buffer >> size;

			for (size_t i = 0; i < size; i++) {
				T element{};
				deserialize(buffer, element);
				container.push_back(element);
			}
		}
	};
}

#endif