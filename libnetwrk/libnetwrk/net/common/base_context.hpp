#ifndef LIBNETWRK_NET_COMMON_BASE_CONTEXT_HPP
#define LIBNETWRK_NET_COMMON_BASE_CONTEXT_HPP

#include <string>

#include "libnetwrk/net/definitions.hpp"
#include "libnetwrk/net/message.hpp"
#include "libnetwrk/net/common/containers/tsdeque.hpp"
#include "libnetwrk/net/common/serialization/serializers/binary_serializer.hpp"

namespace libnetwrk::net::common {
    template <typename command_type,
        typename serializer = libnetwrk::net::common::binary_serializer,
        typename storage = libnetwrk::nothing>
    class base_context {
        public:
            typedef owned_message<command_type, serializer, storage> owned_message_t;

        public:
            const std::string m_name;
            const connection_owner m_owner;

            context_ptr m_context;
            libnetwrk::net::common::tsdeque<owned_message_t> m_incoming_messages;

            base_context(const std::string& name, connection_owner owner) 
                : m_name(name), m_owner(owner) {}

            const char* name() {
                return m_name.c_str();
            }
    };
}

#endif