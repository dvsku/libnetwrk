#pragma once

#include <cstdint>
#include <random>

namespace libnetwrk {
    class auth {
    public:
        using question_t = uint32_t;
        using answer_t   = uint32_t;

    public:
        inline static question_t generate_auth_question() {
            std::random_device seed;
            std::default_random_engine generator(seed());
            std::uniform_int_distribution<uint32_t> distribution(0x0000, 0xFFFF);
            return distribution(generator);
        }

        inline static answer_t generate_auth_answer(question_t question) {
            return question ^ 21205;
        }
    };
}
