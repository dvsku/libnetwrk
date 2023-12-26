#ifndef LIBNETWRK_NET_COMMON_SERIALIZABLE_HPP
#define LIBNETWRK_NET_COMMON_SERIALIZABLE_HPP

#include "libnetwrk/net/definitions.hpp"
#include "libnetwrk/net/type_traits.hpp"

namespace libnetwrk::net::common {
    
    // Forward declare binary_serializer
    struct binary_serializer;
    
    // Forward declare buffer
    template<typename serializer>
    class buffer;

    // Base struct for every user made serializable object
    template <typename serializer = binary_serializer>
    struct serializable {
        using buffer_t = buffer<serializer>;

        // Serializes struct to buffer
        virtual void serialize(buffer_t& buffer) const = 0;

        // Deserializes a struct from a buffer
        virtual void deserialize(buffer_t& serialized) = 0;
    };
}

#endif