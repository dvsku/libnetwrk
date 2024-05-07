#pragma once

#ifdef LIBNETWRK_THROW_INSTEAD_OF_STATIC_ASSERT
#include "libnetwrk/net/shared/exceptions/libnetwrk_exception.hpp"
#endif

#ifdef LIBNETWRK_THROW_INSTEAD_OF_STATIC_ASSERT
#define LIBNETWRK_STATIC_ASSERT_OR_THROW(expr, err_msg)         \
    if(!expr)                                                   \
        throw libnetwrk::libnetwrk_exception(err_msg)
#else
#define LIBNETWRK_STATIC_ASSERT_OR_THROW(expression, err_msg)   \
    static_assert(expression, err_msg)
#endif