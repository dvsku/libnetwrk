#pragma once

#include <cstdint>
#include <random>

namespace libnetwrk {
    class authentication {
    public:
        using request_t  = uint32_t;
        using response_t = uint32_t;

    public:
        inline static request_t generate_request() {
            std::random_device seed;
            std::default_random_engine generator(seed());
            std::uniform_int_distribution<uint32_t> distribution(0x0000, 0xFFFF);
            return distribution(generator);
        }

        inline static response_t generate_response(request_t request) {
            return request ^ 21205;
        }

        inline static bool validate(request_t request, response_t response) {
            response_t correct = generate_response(request);
            return response == correct;
        }
    };
}
