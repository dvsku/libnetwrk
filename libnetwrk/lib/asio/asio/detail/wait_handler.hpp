//
// detail/wait_handler.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_WAIT_HANDLER_HPP
#define ASIO_DETAIL_WAIT_HANDLER_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "lib/asio/asio/detail/config.hpp"
#include "lib/asio/asio/detail/fenced_block.hpp"
#include "lib/asio/asio/detail/handler_alloc_helpers.hpp"
#include "lib/asio/asio/detail/handler_invoke_helpers.hpp"
#include "lib/asio/asio/detail/handler_work.hpp"
#include "lib/asio/asio/detail/memory.hpp"
#include "lib/asio/asio/detail/wait_op.hpp"

#include "lib/asio/asio/detail/push_options.hpp"

namespace asio {
namespace detail {

template <typename Handler, typename IoExecutor>
class wait_handler : public wait_op
{
public:
  ASIO_DEFINE_HANDLER_PTR(wait_handler);

  wait_handler(Handler& h, const IoExecutor& ex)
    : wait_op(&wait_handler::do_complete),
      handler_(ASIO_MOVE_CAST(Handler)(h)),
      io_executor_(ex)
  {
    handler_work<Handler, IoExecutor>::start(handler_, io_executor_);
  }

  static void do_complete(void* owner, operation* base,
      const asio::error_code& /*ec*/,
      std::size_t /*bytes_transferred*/)
  {
    // Take ownership of the handler object.
    wait_handler* h(static_cast<wait_handler*>(base));
    ptr p = { asio::detail::addressof(h->handler_), h, h };
    handler_work<Handler, IoExecutor> w(h->handler_, h->io_executor_);

    ASIO_HANDLER_COMPLETION((*h));

    // Make a copy of the handler so that the memory can be deallocated before
    // the upcall is made. Even if we're not about to make an upcall, a
    // sub-object of the handler may be the true owner of the memory associated
    // with the handler. Consequently, a local copy of the handler is required
    // to ensure that any owning sub-object remains valid until after we have
    // deallocated the memory here.
    detail::binder1<Handler, asio::error_code>
      handler(h->handler_, h->ec_);
    p.h = asio::detail::addressof(handler.handler_);
    p.reset();

    // Make the upcall if required.
    if (owner)
    {
      fenced_block b(fenced_block::half);
      ASIO_HANDLER_INVOCATION_BEGIN((handler.arg1_));
      w.complete(handler, handler.handler_);
      ASIO_HANDLER_INVOCATION_END;
    }
  }

private:
  Handler handler_;
  IoExecutor io_executor_;
};

} // namespace detail
} // namespace asio

#include "lib/asio/asio/detail/pop_options.hpp"

#endif // ASIO_DETAIL_WAIT_HANDLER_HPP
