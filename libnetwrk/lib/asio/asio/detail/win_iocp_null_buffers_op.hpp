//
// detail/win_iocp_null_buffers_op.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_WIN_IOCP_NULL_BUFFERS_OP_HPP
#define ASIO_DETAIL_WIN_IOCP_NULL_BUFFERS_OP_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "lib/asio/asio/detail/config.hpp"

#if defined(ASIO_HAS_IOCP)

#include "lib/asio/asio/detail/bind_handler.hpp"
#include "lib/asio/asio/detail/buffer_sequence_adapter.hpp"
#include "lib/asio/asio/detail/fenced_block.hpp"
#include "lib/asio/asio/detail/handler_alloc_helpers.hpp"
#include "lib/asio/asio/detail/handler_invoke_helpers.hpp"
#include "lib/asio/asio/detail/memory.hpp"
#include "lib/asio/asio/detail/reactor_op.hpp"
#include "lib/asio/asio/detail/socket_ops.hpp"
#include "lib/asio/asio/error.hpp"

#include "lib/asio/asio/detail/push_options.hpp"

namespace asio {
namespace detail {

template <typename Handler, typename IoExecutor>
class win_iocp_null_buffers_op : public reactor_op
{
public:
  ASIO_DEFINE_HANDLER_PTR(win_iocp_null_buffers_op);

  win_iocp_null_buffers_op(socket_ops::weak_cancel_token_type cancel_token,
      Handler& handler, const IoExecutor& io_ex)
    : reactor_op(&win_iocp_null_buffers_op::do_perform,
        &win_iocp_null_buffers_op::do_complete),
      cancel_token_(cancel_token),
      handler_(ASIO_MOVE_CAST(Handler)(handler)),
      io_executor_(io_ex)
  {
    handler_work<Handler, IoExecutor>::start(handler_, io_executor_);
  }

  static status do_perform(reactor_op*)
  {
    return done;
  }

  static void do_complete(void* owner, operation* base,
      const asio::error_code& result_ec,
      std::size_t bytes_transferred)
  {
    asio::error_code ec(result_ec);

    // Take ownership of the operation object.
    win_iocp_null_buffers_op* o(static_cast<win_iocp_null_buffers_op*>(base));
    ptr p = { asio::detail::addressof(o->handler_), o, o };
    handler_work<Handler, IoExecutor> w(o->handler_, o->io_executor_);

    ASIO_HANDLER_COMPLETION((*o));

    // The reactor may have stored a result in the operation object.
    if (o->ec_)
      ec = o->ec_;

    // Map non-portable errors to their portable counterparts.
    if (ec.value() == ERROR_NETNAME_DELETED)
    {
      if (o->cancel_token_.expired())
        ec = asio::error::operation_aborted;
      else
        ec = asio::error::connection_reset;
    }
    else if (ec.value() == ERROR_PORT_UNREACHABLE)
    {
      ec = asio::error::connection_refused;
    }

    // Make a copy of the handler so that the memory can be deallocated before
    // the upcall is made. Even if we're not about to make an upcall, a
    // sub-object of the handler may be the true owner of the memory associated
    // with the handler. Consequently, a local copy of the handler is required
    // to ensure that any owning sub-object remains valid until after we have
    // deallocated the memory here.
    detail::binder2<Handler, asio::error_code, std::size_t>
      handler(o->handler_, ec, bytes_transferred);
    p.h = asio::detail::addressof(handler.handler_);
    p.reset();

    // Make the upcall if required.
    if (owner)
    {
      fenced_block b(fenced_block::half);
      ASIO_HANDLER_INVOCATION_BEGIN((handler.arg1_, handler.arg2_));
      w.complete(handler, handler.handler_);
      ASIO_HANDLER_INVOCATION_END;
    }
  }

private:
  socket_ops::weak_cancel_token_type cancel_token_;
  Handler handler_;
  IoExecutor io_executor_;
};

} // namespace detail
} // namespace asio

#include "lib/asio/asio/detail/pop_options.hpp"

#endif // defined(ASIO_HAS_IOCP)

#endif // ASIO_DETAIL_WIN_IOCP_NULL_BUFFERS_OP_HPP
