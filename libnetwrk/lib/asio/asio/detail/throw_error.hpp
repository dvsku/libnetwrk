//
// detail/throw_error.hpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_THROW_ERROR_HPP
#define ASIO_DETAIL_THROW_ERROR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "lib/asio/asio/detail/config.hpp"
#include "lib/asio/asio/error_code.hpp"

#include "lib/asio/asio/detail/push_options.hpp"

namespace asio {
namespace detail {

ASIO_DECL void do_throw_error(const asio::error_code& err);

ASIO_DECL void do_throw_error(const asio::error_code& err,
    const char* location);

inline void throw_error(const asio::error_code& err)
{
  if (err)
    do_throw_error(err);
}

inline void throw_error(const asio::error_code& err,
    const char* location)
{
  if (err)
    do_throw_error(err, location);
}

} // namespace detail
} // namespace asio

#include "lib/asio/asio/detail/pop_options.hpp"

#if defined(ASIO_HEADER_ONLY)
# include "lib/asio/asio/detail/impl/throw_error.ipp"
#endif // defined(ASIO_HEADER_ONLY)

#endif // ASIO_DETAIL_THROW_ERROR_HPP