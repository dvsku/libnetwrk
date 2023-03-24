#ifndef LIBNETWRK_NET_COMMON_TYPE_TRAITS_HPP
#define LIBNETWRK_NET_COMMON_TYPE_TRAITS_HPP

#include <type_traits>
#include <utility>

#include "libnetwrk/net/common/serialization/serializable.hpp"

namespace libnetwrk::net {
	/// <summary>
	/// Check if type is serializable. Type is serializable if it derives from serializable struct.
	/// </summary>
	/// <typeparam name="T">type</typeparam>
	/// <typeparam name="serializer">serializer to use</typeparam>
	template<typename T, typename serializer> 
	constexpr bool is_serializable = 
		std::is_base_of<libnetwrk::net::common::serializable<serializer>, T>::value;

	/// <summary>
	/// Check if function's signature matches a desired signature
	/// </summary>
	/// <typeparam name="F">: function to test</typeparam>
	/// <typeparam name="R">: return value type</typeparam>
	/// <typeparam name="...Args">: argument types</typeparam>
	template<typename F, typename R, typename... Args>
	constexpr bool is_signature =
		std::is_same<decltype(std::function(std::declval<F>())), std::function<R(Args...)>>::value;
}

#endif