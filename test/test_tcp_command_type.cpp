#define LIBNETWRK_THROW_INSTEAD_OF_STATIC_ASSERT
#include "libnetwrk.hpp"
#include "utilities_assert.hpp"

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

    void on_message(owned_message_t& message) override {
        switch (message.message.head.command) {
            case command_t::c2s_ping: {
                message.message >> ping;
                message_t response(command_t::s2c_pong);
                response << std::string("pOnG");
                message.client->send(response);
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

    void on_message(message_t& message) override {
        switch (message.head.command) {
            case command_t::s2c_pong:
                message >> pong;
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

void t_commands_uint8() {
    ASSERT_NOT_THROWS_CTOR(uint8_server server);
    server.start("127.0.0.1", 21205);

    ASSERT_NOT_THROWS_CTOR(uint8_client client);
    client.connect("127.0.0.1", 21205);

    ASSERT_NOT_THROWS_CTOR(uint8_client::message_t message(uint8_client::command_t::c2s_ping));
    message << std::string("PiNg");
    client.send(message);

    server.wait_for_message();
    ASSERT(server.ping == "PiNg");

    client.wait_for_message();
    ASSERT(client.pong == "pOnG");
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

        void on_message(owned_message_t& message) override {
            switch (message.message.head.command) {
                case command_t::c2s_ping: {
                    message.message >> ping;
                    message_t response(command_t::s2c_pong);
                    response << std::string("pOnG");
                    message.client->send(response);
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

        void on_message(message_t& message) override {
            switch (message.head.command) {
                case command_t::s2c_pong:
                    message >> pong;
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

void t_commands_uint16() {
    ASSERT_NOT_THROWS_CTOR(uint16_server server);
    server.start("127.0.0.1", 21205);

    ASSERT_NOT_THROWS_CTOR(uint16_client client);
    client.connect("127.0.0.1", 21205);

    ASSERT_NOT_THROWS_CTOR(uint16_client::message_t message(uint16_client::command_t::c2s_ping));
    message << std::string("PiNg");
    client.send(message);

    server.wait_for_message();
    ASSERT(server.ping == "PiNg");

    client.wait_for_message();
    ASSERT(client.pong == "pOnG");
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

        void on_message(owned_message_t& message) override {
            switch (message.message.head.command) {
                case command_t::c2s_ping: {
                    message.message >> ping;
                    message_t response(command_t::s2c_pong);
                    response << std::string("pOnG");
                    message.client->send(response);
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

        void on_message(message_t& message) override {
            switch (message.head.command) {
                case command_t::s2c_pong:
                    message >> pong;
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

void t_commands_uint32() {
    ASSERT_NOT_THROWS_CTOR(uint32_server server);
    server.start("127.0.0.1", 21205);

    ASSERT_NOT_THROWS_CTOR(uint32_client client);
    client.connect("127.0.0.1", 21205);

    ASSERT_NOT_THROWS_CTOR(uint32_client::message_t message(uint32_client::command_t::c2s_ping));
    message << std::string("PiNg");
    client.send(message);

    server.wait_for_message();
    ASSERT(server.ping == "PiNg");

    client.wait_for_message();
    ASSERT(client.pong == "pOnG");
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

        void on_message(owned_message_t& message) override {
            switch (message.message.head.command) {
                case command_t::c2s_ping: {
                    message.message >> ping;
                    message_t response(command_t::s2c_pong);
                    response << std::string("pOnG");
                    message.client->send(response);
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

        void on_message(message_t& message) override {
            switch (message.head.command) {
                case command_t::s2c_pong:
                    message >> pong;
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

void t_commands_uint64() {
    ASSERT_NOT_THROWS_CTOR(uint64_server server);
    server.start("127.0.0.1", 21205);

    ASSERT_NOT_THROWS_CTOR(uint64_client client);
    client.connect("127.0.0.1", 21205);

    ASSERT_NOT_THROWS_CTOR(uint64_client::message_t message(uint64_client::command_t::c2s_ping));
    message << std::string("PiNg");
    client.send(message);

    server.wait_for_message();
    ASSERT(server.ping == "PiNg");

    client.wait_for_message();
    ASSERT(client.pong == "pOnG");
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

        void on_message(owned_message_t& message) override {
            switch (message.message.head.command) {
                case command_t::c2s_ping: {
                    message.message >> ping;
                    message_t response(command_t::s2c_pong);
                    response << std::string("pOnG");
                    message.client->send(response);
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

        void on_message(message_t& message) override {
            switch (message.head.command) {
                case command_t::s2c_pong:
                    message >> pong;
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

void t_commands_int8() {
    ASSERT_NOT_THROWS_CTOR(int8_server server);
    server.start("127.0.0.1", 21205);

    ASSERT_NOT_THROWS_CTOR(int8_client client);
    client.connect("127.0.0.1", 21205);

    ASSERT_NOT_THROWS_CTOR(int8_client::message_t message(int8_client::command_t::c2s_ping));
    message << std::string("PiNg");
    client.send(message);

    server.wait_for_message();
    ASSERT(server.ping == "PiNg");

    client.wait_for_message();
    ASSERT(client.pong == "pOnG");
}

///////////////////////////////////////////////////////////////////////////////
// unsigned short
///////////////////////////////////////////////////////////////////////////////

enum class commands_int16 : short {
    c2s_ping = -1,
    s2c_pong = 1
};

class int16_server : public tcp_server<commands_int16> {
    public:
        int16_server() : tcp_server() {}

        std::string ping = "";

        void on_message(owned_message_t& message) override {
            switch (message.message.head.command) {
                case command_t::c2s_ping: {
                    message.message >> ping;
                    message_t response(command_t::s2c_pong);
                    response << std::string("pOnG");
                    message.client->send(response);
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

        void on_message(message_t& message) override {
            switch (message.head.command) {
                case command_t::s2c_pong:
                    message >> pong;
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

void t_commands_int16() {
    ASSERT_NOT_THROWS_CTOR(int16_server server);
    server.start("127.0.0.1", 21205);

    ASSERT_NOT_THROWS_CTOR(int16_client client);
    client.connect("127.0.0.1", 21205);

    ASSERT_NOT_THROWS_CTOR(int16_client::message_t message(int16_client::command_t::c2s_ping));
    message << std::string("PiNg");
    client.send(message);

    server.wait_for_message();
    ASSERT(server.ping == "PiNg");

    client.wait_for_message();
    ASSERT(client.pong == "pOnG");
}

///////////////////////////////////////////////////////////////////////////////
// unsigned int
///////////////////////////////////////////////////////////////////////////////

enum class commands_int32 : int {
    c2s_ping = -1,
    s2c_pong = 1
};

class int32_server : public tcp_server<commands_int32> {
    public:
        int32_server() : tcp_server() {}

        std::string ping = "";

        void on_message(owned_message_t& message) override {
            switch (message.message.head.command) {
                case command_t::c2s_ping: {
                    message.message >> ping;
                    message_t response(command_t::s2c_pong);
                    response << std::string("pOnG");
                    message.client->send(response);
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

        void on_message(message_t& message) override {
            switch (message.head.command) {
                case command_t::s2c_pong:
                    message >> pong;
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

void t_commands_int32() {
    ASSERT_NOT_THROWS_CTOR(int32_server server);
    server.start("127.0.0.1", 21205);

    ASSERT_NOT_THROWS_CTOR(int32_client client);
    client.connect("127.0.0.1", 21205);

    ASSERT_NOT_THROWS_CTOR(int32_client::message_t message(int32_client::command_t::c2s_ping));
    message << std::string("PiNg");
    client.send(message);

    server.wait_for_message();
    ASSERT(server.ping == "PiNg");

    client.wait_for_message();
    ASSERT(client.pong == "pOnG");
}

///////////////////////////////////////////////////////////////////////////////
// unsigned long long
///////////////////////////////////////////////////////////////////////////////

enum class commands_int64 : long long {
    c2s_ping = -1,
    s2c_pong = 1
};

class int64_server : public tcp_server<commands_int64> {
    public:
        int64_server() : tcp_server() {}

        std::string ping = "";

        void on_message(owned_message_t& message) override {
            switch (message.message.head.command) {
                case command_t::c2s_ping: {
                    message.message >> ping;
                    message_t response(command_t::s2c_pong);
                    response << std::string("pOnG");
                    message.client->send(response);
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

        void on_message(message_t& message) override {
            switch (message.head.command) {
                case command_t::s2c_pong:
                    message >> pong;
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

void t_commands_int64() {
    ASSERT_NOT_THROWS_CTOR(int64_server server);
    server.start("127.0.0.1", 21205);

    ASSERT_NOT_THROWS_CTOR(int64_client client);
    client.connect("127.0.0.1", 21205);

    ASSERT_NOT_THROWS_CTOR(int64_client::message_t message(int64_client::command_t::c2s_ping));
    message << std::string("PiNg");
    client.send(message);

    server.wait_for_message();
    ASSERT(server.ping == "PiNg");

    client.wait_for_message();
    ASSERT(client.pong == "pOnG");
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        t_commands_uint8();
        t_commands_uint16();
        t_commands_uint32();
        t_commands_uint64();
        t_commands_int8();
        t_commands_int16();
        t_commands_int32();
        t_commands_int64();
    }
    else {
        switch (std::stoi(argv[1])) {
            case 0: t_commands_uint8();  break;
            case 1: t_commands_uint16(); break;
            case 2: t_commands_uint32(); break;
            case 3: t_commands_uint64(); break;
            case 4: t_commands_int8();   break;
            case 5: t_commands_int16();  break;
            case 6: t_commands_int32();  break;
            case 7: t_commands_int64();  break;    
            default: break;
        }
    }

    return 0;
}