#ifndef LIBNETWRK_NET_COMMON_SERIALIZATION_TYPE_TRAITS_H
#define LIBNETWRK_NET_COMMON_SERIALIZATION_TYPE_TRAITS_H

#include <type_traits>

#include "serializable.hpp"

namespace libnetwrk::net::common {
	// Type trait for every object deriving from serializable struct
	template<typename T> struct is_serializable {
		static constexpr bool value = std::is_base_of<serializable, T>::value;
	};
}

#endif