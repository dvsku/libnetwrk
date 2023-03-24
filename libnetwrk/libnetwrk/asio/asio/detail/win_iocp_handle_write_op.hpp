//
// detail/win_iocp_handle_write_op.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
// Copyright (c) 2008 Rep Invariant Systems, Inc. (info@repinvariant.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_WIN_IOCP_HANDLE_WRITE_OP_HPP
#define ASIO_DETAIL_WIN_IOCP_HANDLE_WRITE_OP_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "libnetwrk/asio/asio/detail/config.hpp"

#if defined(ASIO_HAS_IOCP)

#include "libnetwrk/asio/asio/error.hpp"
#include "libnetwrk/asio/asio/detail/bind_handler.hpp"
#include "libnetwrk/asio/asio/detail/buffer_sequence_adapter.hpp"
#include "libnetwrk/asio/asio/detail/fenced_block.hpp"
#include "libnetwrk/asio/asio/detail/handler_alloc_helpers.hpp"
#include "libnetwrk/asio/asio/detail/handler_invoke_helpers.hpp"
#include "libnetwrk/asio/asio/detail/memory.hpp"
#include "libnetwrk/asio/asio/detail/operation.hpp"

#include "libnetwrk/asio/asio/detail/push_options.hpp"

namespace asio {
namespace detail {

template <typename ConstBufferSequence, typename Handler, typename IoExecutor>
class win_iocp_handle_write_op : public operation
{
public:
  ASIO_DEFINE_HANDLER_PTR(win_iocp_handle_write_op);

  win_iocp_handle_write_op(const ConstBufferSequence& buffers,
      Handler& handler, const IoExecutor& io_ex)
    : operation(&win_iocp_handle_write_op::do_complete),
      buffers_(buffers),
      handler_(ASIO_MOVE_CAST(Handler)(handler)),
      io_executor_(io_ex)
  {
    handler_work<Handler, IoExecutor>::start(handler_, io_executor_);
  }

  static void do_complete(void* owner, operation* base,
      const asio::error_code& ec, std::size_t bytes_transferred)
  {
    // Take ownership of the operation object.
    win_iocp_handle_write_op* o(static_cast<win_iocp_handle_write_op*>(base));
    ptr p = { asio::detail::addressof(o->handler_), o, o };
    handler_work<Handler, IoExecutor> w(o->handler_, o->io_executor_);

    ASIO_HANDLER_COMPLETION((*o));

#if defined(ASIO_ENABLE_BUFFER_DEBUGGING)
    if (owner)
    {
      // Check whether buffers are still valid.
      buffer_sequence_adapter<asio::const_buffer,
          ConstBufferSequence>::validate(o->buffers_);
    }
#endif // defined(ASIO_ENABLE_BUFFER_DEBUGGING)

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
  ConstBufferSequence buffers_;
  Handler handler_;
  IoExecutor io_executor_;
};

} // namespace detail
} // namespace asio

#include "libnetwrk/asio/asio/detail/pop_options.hpp"

#endif // defined(ASIO_HAS_IOCP)

#endif // ASIO_DETAIL_WIN_IOCP_HANDLE_WRITE_OP_HPP