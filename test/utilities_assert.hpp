#ifndef UTILITIES_ASSERT_HPP
#define UTILITIES_ASSERT_HPP

#include <cassert>

#define ASSERT_THROWS(expression) {                                     \
    do {                                                                \
        bool cpputExceptionThrown_ = false;                             \
        try {                                                           \
            expression;                                                 \
        }                                                               \
        catch (...) {                                                   \
            cpputExceptionThrown_ = true;                               \
        }                                                               \
                                                                        \
        assert(cpputExceptionThrown_ == true);                          \
        break;                                                          \
    } while (false);                                                    \
}

#define ASSERT_NOT_THROWS(expression) {                                 \
    do {                                                                \
        bool cpputExceptionThrown_ = false;                             \
        try {                                                           \
            expression;                                                 \
        }                                                               \
        catch (...) {                                                   \
            cpputExceptionThrown_ = true;                               \
        }                                                               \
                                                                        \
        assert(cpputExceptionThrown_ == true);                          \
        break;                                                          \
    } while (false);                                                    \
}

#endif
