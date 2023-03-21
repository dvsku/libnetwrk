//
// ts/executor.hpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_TS_EXECUTOR_HPP
#define ASIO_TS_EXECUTOR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "libnetwrk/asio/asio/async_result.hpp"
#include "libnetwrk/asio/asio/associated_allocator.hpp"
#include "libnetwrk/asio/asio/execution_context.hpp"
#include "libnetwrk/asio/asio/is_executor.hpp"
#include "libnetwrk/asio/asio/associated_executor.hpp"
#include "libnetwrk/asio/asio/bind_executor.hpp"
#include "libnetwrk/asio/asio/executor_work_guard.hpp"
#include "libnetwrk/asio/asio/system_executor.hpp"
#include "libnetwrk/asio/asio/executor.hpp"
#include "libnetwrk/asio/asio/dispatch.hpp"
#include "libnetwrk/asio/asio/post.hpp"
#include "libnetwrk/asio/asio/defer.hpp"
#include "libnetwrk/asio/asio/strand.hpp"
#include "libnetwrk/asio/asio/packaged_task.hpp"
#include "libnetwrk/asio/asio/use_future.hpp"

#endif // ASIO_TS_EXECUTOR_HPP
