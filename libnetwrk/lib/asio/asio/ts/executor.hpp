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

#include "lib/asio/asio/async_result.hpp"
#include "lib/asio/asio/associated_allocator.hpp"
#include "lib/asio/asio/execution_context.hpp"
#include "lib/asio/asio/is_executor.hpp"
#include "lib/asio/asio/associated_executor.hpp"
#include "lib/asio/asio/bind_executor.hpp"
#include "lib/asio/asio/executor_work_guard.hpp"
#include "lib/asio/asio/system_executor.hpp"
#include "lib/asio/asio/executor.hpp"
#include "lib/asio/asio/dispatch.hpp"
#include "lib/asio/asio/post.hpp"
#include "lib/asio/asio/defer.hpp"
#include "lib/asio/asio/strand.hpp"
#include "lib/asio/asio/packaged_task.hpp"
#include "lib/asio/asio/use_future.hpp"

#endif // ASIO_TS_EXECUTOR_HPP
