#define LIBNETWRK_THROW_INSTEAD_OF_STATIC_ASSERT
#include <libnetwrk.hpp>
#include <gtest/gtest.h>

#include <thread>
#include <chrono>

using namespace libnetwrk::tcp;
using namespace libnetwrk;

///////////////////////////////////////////////////////////////////////////////
// unsigned char
///////////////////////////////////////////////////////////////////////////////

enum class commands_uint8 : unsigned char {
    c2s_ping = 0x01,
    s2c_pong = 0x02
};

class uint8_server : public tcp_server<commands_uint8> {
public:
    uint8_server() : tcp_server() {}

    std::string ping = "";

    void ev_message(owned_message_t& message) override {
        switch (message.msg.head.command) {
            case command_t::c2s_ping: {
                message.msg >> ping;
                message_t response(command_t::s2c_pong);
                response << std::string("pOnG");
                message.sender->send(response);
                break;
            }
            default: break;
        }
    }

    void wait_for_message(const int timeout = 30) {
        int tries = 0;
        while (tries < timeout) {
            if (process_message()) break;

            tries++;
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
};

class uint8_client : public tcp_client<commands_uint8> {
public:
    uint8_client() : tcp_client() {}

    std::string pong = "";

    void ev_message(owned_message_t& message) override {
        switch (message.msg.head.command) {
            case command_t::s2c_pong:
                message.msg >> pong;
                break;
            default: break;
        }
    }

    void wait_for_message(const int timeout = 30) {
        int tries = 0;
        while (tries < timeout) {
            if (process_message()) break;

            tries++;
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
};

TEST(commands, uint8) {
    ASSERT_NO_THROW(uint8_server server);
    uint8_server server;
    server.start("127.0.0.1", 21205);

    ASSERT_NO_THROW(uint8_client client);
    uint8_client client;
    client.connect("127.0.0.1", 21205);

    ASSERT_NO_THROW(uint8_client::message_t message(uint8_client::command_t::c2s_ping));
    uint8_client::message_t message(uint8_client::command_t::c2s_ping);
    message << std::string("PiNg");
    client.send(message);

    server.wait_for_message();
    EXPECT_TRUE(server.ping == "PiNg");

    client.wait_for_message();
    EXPECT_TRUE(client.pong == "pOnG");
}

///////////////////////////////////////////////////////////////////////////////
// unsigned short
///////////////////////////////////////////////////////////////////////////////

enum class commands_uint16 : unsigned short {
    c2s_ping = 0x01,
    s2c_pong = 0x02
};

class uint16_server : public tcp_server<commands_uint16> {
    public:
        uint16_server() : tcp_server() {}

        std::string ping = "";

        void ev_message(owned_message_t& message) override {
            switch (message.msg.head.command) {
                case command_t::c2s_ping: {
                    message.msg >> ping;
                    message_t response(command_t::s2c_pong);
                    response << std::string("pOnG");
                    message.sender->send(response);
                    break;
                }
                default: break;
            }
        }

        void wait_for_message(const int timeout = 30) {
            int tries = 0;
            while (tries < timeout) {
                if (process_message()) break;

                tries++;
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        }
};

class uint16_client : public tcp_client<commands_uint16> {
    public:
        uint16_client() : tcp_client() {}

        std::string pong = "";

        void ev_message(owned_message_t& message) override {
            switch (message.msg.head.command) {
                case command_t::s2c_pong:
                    message.msg >> pong;
                    break;
                default: break;
            }
        }

        void wait_for_message(const int timeout = 30) {
            int tries = 0;
            while (tries < timeout) {
                if (process_message()) break;

                tries++;
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        }
};

TEST(commands, uint16) {
    ASSERT_NO_THROW(uint16_server server);
    uint16_server server;
    server.start("127.0.0.1", 21205);

    ASSERT_NO_THROW(uint16_client client);
    uint16_client client;
    client.connect("127.0.0.1", 21205);

    ASSERT_NO_THROW(uint16_client::message_t message(uint16_client::command_t::c2s_ping));
    uint16_client::message_t message(uint16_client::command_t::c2s_ping);
    message << std::string("PiNg");
    client.send(message);

    server.wait_for_message();
    EXPECT_TRUE(server.ping == "PiNg");

    client.wait_for_message();
    EXPECT_TRUE(client.pong == "pOnG");
}

///////////////////////////////////////////////////////////////////////////////
// unsigned int
///////////////////////////////////////////////////////////////////////////////

enum class commands_uint32 : unsigned int {
    c2s_ping = 0x01,
    s2c_pong = 0x02
};

class uint32_server : public tcp_server<commands_uint32> {
    public:
        uint32_server() : tcp_server() {}

        std::string ping = "";

        void ev_message(owned_message_t& message) override {
            switch (message.msg.head.command) {
                case command_t::c2s_ping: {
                    message.msg >> ping;
                    message_t response(command_t::s2c_pong);
                    response << std::string("pOnG");
                    message.sender->send(response);
                    break;
                }
                default: break;
            }
        }

        void wait_for_message(const int timeout = 30) {
            int tries = 0;
            while (tries < timeout) {
                if (process_message()) break;

                tries++;
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        }
};

class uint32_client : public tcp_client<commands_uint32> {
    public:
        uint32_client() : tcp_client() {}

        std::string pong = "";

        void ev_message(owned_message_t& message) override {
            switch (message.msg.head.command) {
                case command_t::s2c_pong:
                    message.msg >> pong;
                    break;
                default: break;
            }
        }

        void wait_for_message(const int timeout = 30) {
            int tries = 0;
            while (tries < timeout) {
                if (process_message()) break;

                tries++;
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        }
};

TEST(commands, uint32) {
    ASSERT_NO_THROW(uint32_server server);
    uint32_server server;
    server.start("127.0.0.1", 21205);

    ASSERT_NO_THROW(uint32_client client);
    uint32_client client;
    client.connect("127.0.0.1", 21205);

    ASSERT_NO_THROW(uint32_client::message_t message(uint32_client::command_t::c2s_ping));
    uint32_client::message_t message(uint32_client::command_t::c2s_ping);
    message << std::string("PiNg");
    client.send(message);

    server.wait_for_message();
    EXPECT_TRUE(server.ping == "PiNg");

    client.wait_for_message();
    EXPECT_TRUE(client.pong == "pOnG");
}

///////////////////////////////////////////////////////////////////////////////
// unsigned long long
///////////////////////////////////////////////////////////////////////////////

enum class commands_uint64 : unsigned long long {
    c2s_ping = 0x01,
    s2c_pong = 0x02
};

class uint64_server : public tcp_server<commands_uint64> {
    public:
        uint64_server() : tcp_server() {}

        std::string ping = "";

        void ev_message(owned_message_t& message) override {
            switch (message.msg.head.command) {
                case command_t::c2s_ping: {
                    message.msg >> ping;
                    message_t response(command_t::s2c_pong);
                    response << std::string("pOnG");
                    message.sender->send(response);
                    break;
                }
                default: break;
            }
        }

        void wait_for_message(const int timeout = 30) {
            int tries = 0;
            while (tries < timeout) {
                if (process_message()) break;

                tries++;
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        }
};

class uint64_client : public tcp_client<commands_uint64> {
    public:
        uint64_client() : tcp_client() {}

        std::string pong = "";

        void ev_message(owned_message_t& message) override {
            switch (message.msg.head.command) {
                case command_t::s2c_pong:
                    message.msg >> pong;
                    break;
                default: break;
            }
        }

        void wait_for_message(const int timeout = 30) {
            int tries = 0;
            while (tries < timeout) {
                if (process_message()) break;

                tries++;
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        }
};

TEST(commands, uint64) {
    ASSERT_NO_THROW(uint64_server server);
    uint64_server server;
    server.start("127.0.0.1", 21205);

    ASSERT_NO_THROW(uint64_client client);
    uint64_client client;
    client.connect("127.0.0.1", 21205);

    ASSERT_NO_THROW(uint64_client::message_t message(uint64_client::command_t::c2s_ping));
    uint64_client::message_t message(uint64_client::command_t::c2s_ping);
    message << std::string("PiNg");
    client.send(message);

    server.wait_for_message();
    EXPECT_TRUE(server.ping == "PiNg");

    client.wait_for_message();
    EXPECT_TRUE(client.pong == "pOnG");
}

///////////////////////////////////////////////////////////////////////////////
// char
///////////////////////////////////////////////////////////////////////////////

enum class commands_int8 : char {
    c2s_ping = -1,
    s2c_pong = 1
};

class int8_server : public tcp_server<commands_int8> {
    public:
        int8_server() : tcp_server() {}

        std::string ping = "";

        void ev_message(owned_message_t& message) override {
            switch (message.msg.head.command) {
                case command_t::c2s_ping: {
                    message.msg >> ping;
                    message_t response(command_t::s2c_pong);
                    response << std::string("pOnG");
                    message.sender->send(response);
                    break;
                }
                default: break;
            }
        }

        void wait_for_message(const int timeout = 30) {
            int tries = 0;
            while (tries < timeout) {
                if (process_message()) break;

                tries++;
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        }
};

class int8_client : public tcp_client<commands_int8> {
    public:
        int8_client() : tcp_client() {}

        std::string pong = "";

        void ev_message(owned_message_t& message) override {
            switch (message.msg.head.command) {
                case command_t::s2c_pong:
                    message.msg >> pong;
                    break;
                default: break;
            }
        }

        void wait_for_message(const int timeout = 30) {
            int tries = 0;
            while (tries < timeout) {
                if (process_message()) break;

                tries++;
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        }
};

TEST(commands, int8) {
    ASSERT_NO_THROW(int8_server server);
    int8_server server;
    server.start("127.0.0.1", 21205);

    ASSERT_NO_THROW(int8_client client);
    int8_client client;
    client.connect("127.0.0.1", 21205);

    ASSERT_NO_THROW(int8_client::message_t message(int8_client::command_t::c2s_ping));
    int8_client::message_t message(int8_client::command_t::c2s_ping);
    message << std::string("PiNg");
    client.send(message);

    server.wait_for_message();
    EXPECT_TRUE(server.ping == "PiNg");

    client.wait_for_message();
    EXPECT_TRUE(client.pong == "pOnG");
}

///////////////////////////////////////////////////////////////////////////////
// short
///////////////////////////////////////////////////////////////////////////////

enum class commands_int16 : short {
    c2s_ping = -1,
    s2c_pong = 1
};

class int16_server : public tcp_server<commands_int16> {
    public:
        int16_server() : tcp_server() {}

        std::string ping = "";

        void ev_message(owned_message_t& message) override {
            switch (message.msg.head.command) {
                case command_t::c2s_ping: {
                    message.msg >> ping;
                    message_t response(command_t::s2c_pong);
                    response << std::string("pOnG");
                    message.sender->send(response);
                    break;
                }
                default: break;
            }
        }

        void wait_for_message(const int timeout = 30) {
            int tries = 0;
            while (tries < timeout) {
                if (process_message()) break;

                tries++;
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        }
};

class int16_client : public tcp_client<commands_int16> {
    public:
        int16_client() : tcp_client() {}

        std::string pong = "";

        void ev_message(owned_message_t& message) override {
            switch (message.msg.head.command) {
                case command_t::s2c_pong:
                    message.msg >> pong;
                    break;
                default: break;
            }
        }

        void wait_for_message(const int timeout = 30) {
            int tries = 0;
            while (tries < timeout) {
                if (process_message()) break;

                tries++;
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        }
};

TEST(commands, int16) {
    ASSERT_NO_THROW(int16_server server);
    int16_server server;
    server.start("127.0.0.1", 21205);

    ASSERT_NO_THROW(int16_client client);
    int16_client client;
    client.connect("127.0.0.1", 21205);

    ASSERT_NO_THROW(int16_client::message_t message(int16_client::command_t::c2s_ping));
    int16_client::message_t message(int16_client::command_t::c2s_ping);
    message << std::string("PiNg");
    client.send(message);

    server.wait_for_message();
    EXPECT_TRUE(server.ping == "PiNg");

    client.wait_for_message();
    EXPECT_TRUE(client.pong == "pOnG");
}

///////////////////////////////////////////////////////////////////////////////
// int
///////////////////////////////////////////////////////////////////////////////

enum class commands_int32 : int {
    c2s_ping = -1,
    s2c_pong = 1
};

class int32_server : public tcp_server<commands_int32> {
    public:
        int32_server() : tcp_server() {}

        std::string ping = "";

        void ev_message(owned_message_t& message) override {
            switch (message.msg.head.command) {
                case command_t::c2s_ping: {
                    message.msg >> ping;
                    message_t response(command_t::s2c_pong);
                    response << std::string("pOnG");
                    message.sender->send(response);
                    break;
                }
                default: break;
            }
        }

        void wait_for_message(const int timeout = 30) {
            int tries = 0;
            while (tries < timeout) {
                if (process_message()) break;

                tries++;
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        }
};

class int32_client : public tcp_client<commands_int32> {
    public:
        int32_client() : tcp_client() {}

        std::string pong = "";

        void ev_message(owned_message_t& message) override {
            switch (message.msg.head.command) {
                case command_t::s2c_pong:
                    message.msg >> pong;
                    break;
                default: break;
            }
        }

        void wait_for_message(const int timeout = 30) {
            int tries = 0;
            while (tries < timeout) {
                if (process_message()) break;

                tries++;
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        }
};

TEST(commands, int32) {
    ASSERT_NO_THROW(int32_server server);
    int32_server server;
    server.start("127.0.0.1", 21205);

    ASSERT_NO_THROW(int32_client client);
    int32_client client;
    client.connect("127.0.0.1", 21205);

    ASSERT_NO_THROW(int32_client::message_t message(int32_client::command_t::c2s_ping));
    int32_client::message_t message(int32_client::command_t::c2s_ping);
    message << std::string("PiNg");
    client.send(message);

    server.wait_for_message();
    EXPECT_TRUE(server.ping == "PiNg");

    client.wait_for_message();
    EXPECT_TRUE(client.pong == "pOnG");
}

///////////////////////////////////////////////////////////////////////////////
// long long
///////////////////////////////////////////////////////////////////////////////

enum class commands_int64 : long long {
    c2s_ping = -1,
    s2c_pong = 1
};

class int64_server : public tcp_server<commands_int64> {
    public:
        int64_server() : tcp_server() {}

        std::string ping = "";

        void ev_message(owned_message_t& message) override {
            switch (message.msg.head.command) {
                case command_t::c2s_ping: {
                    message.msg >> ping;
                    message_t response(command_t::s2c_pong);
                    response << std::string("pOnG");
                    message.sender->send(response);
                    break;
                }
                default: break;
            }
        }

        void wait_for_message(const int timeout = 30) {
            int tries = 0;
            while (tries < timeout) {
                if (process_message()) break;

                tries++;
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        }
};

class int64_client : public tcp_client<commands_int64> {
    public:
        int64_client() : tcp_client() {}

        std::string pong = "";

        void ev_message(owned_message_t& message) override {
            switch (message.msg.head.command) {
                case command_t::s2c_pong:
                    message.msg >> pong;
                    break;
                default: break;
            }
        }

        void wait_for_message(const int timeout = 30) {
            int tries = 0;
            while (tries < timeout) {
                if (process_message()) break;

                tries++;
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        }
};

TEST(commands, int64) {
    ASSERT_NO_THROW(int64_server server);
    int64_server server;
    server.start("127.0.0.1", 21205);

    ASSERT_NO_THROW(int64_client client);
    int64_client client;
    client.connect("127.0.0.1", 21205);

    ASSERT_NO_THROW(int64_client::message_t message(int64_client::command_t::c2s_ping));
    int64_client::message_t message(int64_client::command_t::c2s_ping);
    message << std::string("PiNg");
    client.send(message);

    server.wait_for_message();
    EXPECT_TRUE(server.ping == "PiNg");

    client.wait_for_message();
    EXPECT_TRUE(client.pong == "pOnG");
}
