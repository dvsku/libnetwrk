#ifndef LIBNETWRK_NET_COMMON_TYPE_TRAITS_HPP
#define LIBNETWRK_NET_COMMON_TYPE_TRAITS_HPP

#include <type_traits>
#include <utility>

#include "libnetwrk/net/common/serialization/serializable.hpp"

namespace libnetwrk::net::common {
	/// <summary>
	/// Type is serializable if it is a derived from serializable
	/// </summary>
	/// <typeparam name="T">type</typeparam>
	/// <typeparam name="serializer">serializer to use</typeparam>
	template<typename T, typename serializer> 
	struct is_serializable {
		static constexpr bool value = std::is_base_of<serializable<serializer>, T>::value;
	};
}

#endif