#ifndef LIBNETWRK_NET_MACROS_HPP
#define LIBNETWRK_NET_MACROS_HPP

// Define supported serialize function for container that has a type.
#define SERIALIZER_SUPPORTED_SERIALIZE_SINGLE(type)											\
	template<typename TValue>																\
	static void serialize(buffer_t& buffer, const type<TValue>& container)					\

// Define supported deserialize function for container that has a type.
#define SERIALIZER_SUPPORTED_DESERIALIZE_SINGLE(type)										\
	template<typename TValue>																\
	static void deserialize(buffer_t& buffer, type<TValue>& container)						\

// Define supported serialize function for container that has a key-value pair.
#define SERIALIZER_SUPPORTED_SERIALIZE_PAIR(type)											\
	template<typename TKey, typename TValue>												\
	static void serialize(buffer_t& buffer, const type<TKey, TValue>& container)			\

// Define supported deserialize function for container that has a key-value pair.
#define SERIALIZER_SUPPORTED_DESERIALIZE_PAIR(type)											\
	template<typename TKey, typename TValue>												\
	static void deserialize(buffer_t& buffer, type<TKey, TValue>& container)				\

#ifdef LIBNETWRK_THROW_INSTEAD_OF_STATIC_ASSERT
	#include "libnetwrk/net/common/exceptions/libnetwrk_exception.hpp"

	// Defines unsupported serialize and deserialize functions for containers that have a type.
	// Functions will throw an exception during runtime if called.
	// Useful for testing, not recommended for use outside of tests.
	#define SERIALIZER_UNSUPPORTED_SINGLE(type)																			\
		template<typename TValue>																						\
		static void serialize(buffer_t& buffer, const type<TValue>& container) {										\
			throw libnetwrk::net::common::libnetwrk_exception("serialize() doesn't support " #type " containers");		\
		}																												\
																														\
		template<typename TValue>																						\
		static void deserialize(buffer_t& buffer, type<TValue>& container) {											\
			throw libnetwrk::net::common::libnetwrk_exception("deserialize() doesn't support " #type " containers");	\
		}

	// Defines unsupported serialize and deserialize functions for containers that have a key-value pair.
	// Functions will throw an exception during runtime if called.
	// Useful for testing, not recommended for use outside of tests.
	#define SERIALIZER_UNSUPPORTED_PAIR(type)																			\
		template<typename TKey, typename TValue>																		\
		static void serialize(buffer_t& buffer, const type<TKey, TValue>& container) {									\
			throw libnetwrk::net::common::libnetwrk_exception("serialize() doesn't support " #type " containers");		\
		}																												\
																														\
		template<typename TKey, typename TValue>																		\
		static void deserialize(buffer_t& buffer, type<TKey, TValue>& container) {										\
			throw libnetwrk::net::common::libnetwrk_exception("deserialize() doesn't support " #type " containers");	\
		}
#else
	template <typename...>
	inline constexpr bool always_false = false;

	// Defines unsupported serialize and deserialize functions for containers that have a type.
	// Functions will output an error during compilation if referenced.
	#define SERIALIZER_UNSUPPORTED_SINGLE(type)		 																\
		template<typename TValue>																					\
		static void serialize(buffer_t& buffer, const type<TValue>& container) {									\
			static_assert(always_false<type<TValue>>, "serialize() doesn't support " #type " containers");			\
		}																											\
																													\
		template<typename TValue>																					\
		static void deserialize(buffer_t& buffer, type<TValue>& container) {										\
			static_assert(always_false<type<TValue>>, "deserialize() doesn't support " #type " containers");		\
		}

	// Defines unsupported serialize and deserialize functions for containers that have a key-value pair.
	// Functions will output an error during compilation if referenced.
	#define SERIALIZER_UNSUPPORTED_PAIR(type)																		\
		template<typename TKey, typename TValue>																	\
		static void serialize(buffer_t& buffer, const type<TKey, TValue>& container) {								\
			static_assert(always_false<type<TKey, TValue>>, "serialize() doesn't support " #type " containers");	\
		}																											\
																													\
		template<typename TKey, typename TValue>																	\
		static void deserialize(buffer_t& buffer, type<TKey, TValue>& container) {									\
			static_assert(always_false<type<TKey, TValue>>, "deserialize() doesn't support " #type " containers");	\
		}
#endif

#endif