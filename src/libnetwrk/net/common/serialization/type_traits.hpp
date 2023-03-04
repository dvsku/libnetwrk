#ifndef LIBNETWRK_NET_COMMON_TYPE_TRAITS_HPP
#define LIBNETWRK_NET_COMMON_TYPE_TRAITS_HPP

#include <type_traits>

#include "libnetwrk/net/common/serialization/serializable.hpp"

namespace libnetwrk::net::common {
	// Type trait for every object deriving from serializable struct
	template<typename T> struct is_serializable {
		static constexpr bool value = std::is_base_of<serializable, T>::value;
	};
}

#endif