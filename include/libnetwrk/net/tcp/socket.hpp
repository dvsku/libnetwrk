#pragma once

#include "libnetwrk/dependencies/asio/asio.hpp"
#include "libnetwrk/net/core/containers/buffer.hpp"

#include <cstdint>
#include <cstddef>
#include <system_error>
#include <functional>

namespace libnetwrk::tcp {
    class socket {
    public:
        using native_socket_t  = asio::ip::tcp::socket;
        using read_callback_t  = std::function<void(std::error_code, std::size_t)>;
        using write_callback_t = std::function<void(std::error_code, std::size_t)>;

    public:
        socket(native_socket_t socket)
            : m_socket(std::move(socket)) {}

    public:
        /*
            Get socket ip address
        */
        std::string get_ip() {
            return m_socket.remote_endpoint().address().to_string();
        }

        /*
            Get socket port
        */
        uint16_t get_port() {
            return m_socket.remote_endpoint().port();
        }

        /*
            Check if socket is connected
        */
        bool is_connected() {
            return m_socket.is_open();
        }

    public:
        void close() {
            m_socket.close();
        }

        template<typename Serialize>
        void async_read(libnetwrk::buffer<Serialize>& buffer, read_callback_t callback) {
            asio::async_read(m_socket, asio::buffer(buffer.data(), buffer.size()), callback);
        }

        template<typename Serialize>
        void async_write(libnetwrk::buffer<Serialize>& buffer, write_callback_t callback) {
            asio::async_write(m_socket, asio::buffer(buffer.data(), buffer.size()), callback);
        }

    private:
        native_socket_t m_socket;
    };
}
