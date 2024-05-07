//
// ts/internet.hpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_TS_INTERNET_HPP
#define ASIO_TS_INTERNET_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "libnetwrk/dependencies/asio/asio/ip/address.hpp"
#include "libnetwrk/dependencies/asio/asio/ip/address_v4.hpp"
#include "libnetwrk/dependencies/asio/asio/ip/address_v4_iterator.hpp"
#include "libnetwrk/dependencies/asio/asio/ip/address_v4_range.hpp"
#include "libnetwrk/dependencies/asio/asio/ip/address_v6.hpp"
#include "libnetwrk/dependencies/asio/asio/ip/address_v6_iterator.hpp"
#include "libnetwrk/dependencies/asio/asio/ip/address_v6_range.hpp"
#include "libnetwrk/dependencies/asio/asio/ip/bad_address_cast.hpp"
#include "libnetwrk/dependencies/asio/asio/ip/basic_endpoint.hpp"
#include "libnetwrk/dependencies/asio/asio/ip/basic_resolver_query.hpp"
#include "libnetwrk/dependencies/asio/asio/ip/basic_resolver_entry.hpp"
#include "libnetwrk/dependencies/asio/asio/ip/basic_resolver_iterator.hpp"
#include "libnetwrk/dependencies/asio/asio/ip/basic_resolver.hpp"
#include "libnetwrk/dependencies/asio/asio/ip/host_name.hpp"
#include "libnetwrk/dependencies/asio/asio/ip/network_v4.hpp"
#include "libnetwrk/dependencies/asio/asio/ip/network_v6.hpp"
#include "libnetwrk/dependencies/asio/asio/ip/tcp.hpp"
#include "libnetwrk/dependencies/asio/asio/ip/udp.hpp"
#include "libnetwrk/dependencies/asio/asio/ip/v6_only.hpp"
#include "libnetwrk/dependencies/asio/asio/ip/unicast.hpp"
#include "libnetwrk/dependencies/asio/asio/ip/multicast.hpp"

#endif // ASIO_TS_INTERNET_HPP
