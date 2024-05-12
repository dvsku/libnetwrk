#define LIBNETWRK_THROW_INSTEAD_OF_STATIC_ASSERT
#include <libnetwrk.hpp>
#include <gtest/gtest.h>

#include <thread>
#include <chrono>

using namespace libnetwrk::tcp;
using namespace libnetwrk;

template<typename T>
struct service_desc {
    using command_t   = T;
    using serialize_t = libnetwrk::bin_serialize;
    using storage_t   = libnetwrk::nothing;
};

template<typename T>
class basic_service : public tcp_service<service_desc<T>> {
public:
    using base_t = tcp_service<service_desc<T>>;

    basic_service() : tcp_service<service_desc<T>>() {}

    std::string ping = "";

    void ev_message(base_t::owned_message_t& message) override {
        switch (message.msg.command()) {
            case base_t::command_t::c2s_ping:
            {
                message.msg >> ping;
                libnetwrk::message<service_desc<T>> response(base_t::command_t::s2c_pong);
                response << std::string("pOnG");
                message.sender->send(response);
                break;
            }
            default: break;
        }
    }
};

template<typename T>
class basic_client : public tcp_client<service_desc<T>> {
public:
    using base_t = tcp_client<service_desc<T>>;

    basic_client() : tcp_client<service_desc<T>>() {}

    std::string pong = "";

    void ev_message(base_t::owned_message_t& message) override {
        switch (message.msg.command()) {
            case base_t::command_t::s2c_pong:
                message.msg >> pong;
                break;
            default: break;
        }
    }
};

///////////////////////////////////////////////////////////////////////////////
// unsigned char
///////////////////////////////////////////////////////////////////////////////

enum class commands_uint8 : unsigned char {
    c2s_ping = 0x01,
    s2c_pong = 0x02
};

TEST(commands, uint8) {
    ASSERT_NO_THROW(basic_service<commands_uint8> service);
    basic_service<commands_uint8> service;
    service.start("127.0.0.1", 21205);

    ASSERT_NO_THROW(basic_client<commands_uint8> client);
    basic_client<commands_uint8> client;
    client.connect("127.0.0.1", 21205);

    basic_client<commands_uint8>::message_t message(basic_client<commands_uint8>::command_t::c2s_ping);
    message << std::string("PiNg");
    client.send(message);

    service.process_messages_async();
    client.process_messages_async();

    std::this_thread::sleep_for(std::chrono::milliseconds(2500));

    EXPECT_TRUE(service.ping == "PiNg");
    EXPECT_TRUE(client.pong == "pOnG");
}

///////////////////////////////////////////////////////////////////////////////
// unsigned short
///////////////////////////////////////////////////////////////////////////////

enum class commands_uint16 : unsigned short {
    c2s_ping = 0x01,
    s2c_pong = 0x02
};

TEST(commands, uint16) {
    ASSERT_NO_THROW(basic_service<commands_uint16> service);
    basic_service<commands_uint16> service;
    service.start("127.0.0.1", 21205);

    ASSERT_NO_THROW(basic_client<commands_uint16> client);
    basic_client<commands_uint16> client;
    client.connect("127.0.0.1", 21205);

    basic_client<commands_uint16>::message_t message(basic_client<commands_uint16>::command_t::c2s_ping);
    message << std::string("PiNg");
    client.send(message);

    service.process_messages_async();
    client.process_messages_async();

    std::this_thread::sleep_for(std::chrono::milliseconds(2500));

    EXPECT_TRUE(service.ping == "PiNg");
    EXPECT_TRUE(client.pong == "pOnG");
}

///////////////////////////////////////////////////////////////////////////////
// unsigned int
///////////////////////////////////////////////////////////////////////////////

enum class commands_uint32 : unsigned int {
    c2s_ping = 0x01,
    s2c_pong = 0x02
};

TEST(commands, uint32) {
    ASSERT_NO_THROW(basic_service<commands_uint32> service);
    basic_service<commands_uint32> service;
    service.start("127.0.0.1", 21205);

    ASSERT_NO_THROW(basic_client<commands_uint32> client);
    basic_client<commands_uint32> client;
    client.connect("127.0.0.1", 21205);

    basic_client<commands_uint32>::message_t message(basic_client<commands_uint32>::command_t::c2s_ping);
    message << std::string("PiNg");
    client.send(message);

    service.process_messages_async();
    client.process_messages_async();

    std::this_thread::sleep_for(std::chrono::milliseconds(2500));

    EXPECT_TRUE(service.ping == "PiNg");
    EXPECT_TRUE(client.pong == "pOnG");
}

///////////////////////////////////////////////////////////////////////////////
// unsigned long long
///////////////////////////////////////////////////////////////////////////////

enum class commands_uint64 : unsigned long long {
    c2s_ping = 0x01,
    s2c_pong = 0x02
};

TEST(commands, uint64) {
    ASSERT_NO_THROW(basic_service<commands_uint64> service);
    basic_service<commands_uint64> service;
    service.start("127.0.0.1", 21205);

    ASSERT_NO_THROW(basic_client<commands_uint64> client);
    basic_client<commands_uint64> client;
    client.connect("127.0.0.1", 21205);

    basic_client<commands_uint64>::message_t message(basic_client<commands_uint64>::command_t::c2s_ping);
    message << std::string("PiNg");
    client.send(message);

    service.process_messages_async();
    client.process_messages_async();

    std::this_thread::sleep_for(std::chrono::milliseconds(2500));

    EXPECT_TRUE(service.ping == "PiNg");
    EXPECT_TRUE(client.pong == "pOnG");
}

///////////////////////////////////////////////////////////////////////////////
// char
///////////////////////////////////////////////////////////////////////////////

enum class commands_int8 : char {
    c2s_ping = -1,
    s2c_pong = 1
};

TEST(commands, int8) {
    ASSERT_NO_THROW(basic_service<commands_int8> service);
    basic_service<commands_int8> service;
    service.start("127.0.0.1", 21205);

    ASSERT_NO_THROW(basic_client<commands_int8> client);
    basic_client<commands_int8> client;
    client.connect("127.0.0.1", 21205);

    basic_client<commands_int8>::message_t message(basic_client<commands_int8>::command_t::c2s_ping);
    message << std::string("PiNg");
    client.send(message);

    service.process_messages_async();
    client.process_messages_async();

    std::this_thread::sleep_for(std::chrono::milliseconds(2500));

    EXPECT_TRUE(service.ping == "PiNg");
    EXPECT_TRUE(client.pong == "pOnG");
}

///////////////////////////////////////////////////////////////////////////////
// short
///////////////////////////////////////////////////////////////////////////////

enum class commands_int16 : short {
    c2s_ping = -1,
    s2c_pong = 1
};

TEST(commands, int16) {
    ASSERT_NO_THROW(basic_service<commands_int16> service);
    basic_service<commands_int16> service;
    service.start("127.0.0.1", 21205);

    ASSERT_NO_THROW(basic_client<commands_int16> client);
    basic_client<commands_int16> client;
    client.connect("127.0.0.1", 21205);

    basic_client<commands_int16>::message_t message(basic_client<commands_int16>::command_t::c2s_ping);
    message << std::string("PiNg");
    client.send(message);

    service.process_messages_async();
    client.process_messages_async();

    std::this_thread::sleep_for(std::chrono::milliseconds(2500));

    EXPECT_TRUE(service.ping == "PiNg");
    EXPECT_TRUE(client.pong == "pOnG");
}

///////////////////////////////////////////////////////////////////////////////
// int
///////////////////////////////////////////////////////////////////////////////

enum class commands_int32 : int {
    c2s_ping = -1,
    s2c_pong = 1
};

TEST(commands, int32) {
    ASSERT_NO_THROW(basic_service<commands_int32> service);
    basic_service<commands_int32> service;
    service.start("127.0.0.1", 21205);

    ASSERT_NO_THROW(basic_client<commands_int32> client);
    basic_client<commands_int32> client;
    client.connect("127.0.0.1", 21205);

    basic_client<commands_int32>::message_t message(basic_client<commands_int32>::command_t::c2s_ping);
    message << std::string("PiNg");
    client.send(message);

    service.process_messages_async();
    client.process_messages_async();

    std::this_thread::sleep_for(std::chrono::milliseconds(2500));

    EXPECT_TRUE(service.ping == "PiNg");
    EXPECT_TRUE(client.pong == "pOnG");
}

///////////////////////////////////////////////////////////////////////////////
// long long
///////////////////////////////////////////////////////////////////////////////

enum class commands_int64 : long long {
    c2s_ping = -1,
    s2c_pong = 1
};

TEST(commands, int64) {
    ASSERT_NO_THROW(basic_service<commands_int64> service);
    basic_service<commands_int64> service;
    service.start("127.0.0.1", 21205);

    ASSERT_NO_THROW(basic_client<commands_int64> client);
    basic_client<commands_int64> client;
    client.connect("127.0.0.1", 21205);

    basic_client<commands_int64>::message_t message(basic_client<commands_int64>::command_t::c2s_ping);
    message << std::string("PiNg");
    client.send(message);

    service.process_messages_async();
    client.process_messages_async();

    std::this_thread::sleep_for(std::chrono::milliseconds(2500));

    EXPECT_TRUE(service.ping == "PiNg");
    EXPECT_TRUE(client.pong == "pOnG");
}
