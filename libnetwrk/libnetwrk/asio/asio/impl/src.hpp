//
// impl/src.hpp
// ~~~~~~~~~~~~
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_IMPL_SRC_HPP
#define ASIO_IMPL_SRC_HPP

#define ASIO_SOURCE

#include "libnetwrk/asio/asio/detail/config.hpp"

#if defined(ASIO_HEADER_ONLY)
# error Do not compile Asio library source with ASIO_HEADER_ONLY defined
#endif

#include "libnetwrk/asio/asio/impl/error.ipp"
#include "libnetwrk/asio/asio/impl/error_code.ipp"
#include "libnetwrk/asio/asio/impl/execution_context.ipp"
#include "libnetwrk/asio/asio/impl/executor.ipp"
#include "libnetwrk/asio/asio/impl/handler_alloc_hook.ipp"
#include "libnetwrk/asio/asio/impl/io_context.ipp"
#include "libnetwrk/asio/asio/impl/serial_port_base.ipp"
#include "libnetwrk/asio/asio/impl/system_context.ipp"
#include "libnetwrk/asio/asio/impl/thread_pool.ipp"
#include "libnetwrk/asio/asio/detail/impl/buffer_sequence_adapter.ipp"
#include "libnetwrk/asio/asio/detail/impl/descriptor_ops.ipp"
#include "libnetwrk/asio/asio/detail/impl/dev_poll_reactor.ipp"
#include "libnetwrk/asio/asio/detail/impl/epoll_reactor.ipp"
#include "libnetwrk/asio/asio/detail/impl/eventfd_select_interrupter.ipp"
#include "libnetwrk/asio/asio/detail/impl/handler_tracking.ipp"
#include "libnetwrk/asio/asio/detail/impl/kqueue_reactor.ipp"
#include "libnetwrk/asio/asio/detail/impl/null_event.ipp"
#include "libnetwrk/asio/asio/detail/impl/pipe_select_interrupter.ipp"
#include "libnetwrk/asio/asio/detail/impl/posix_event.ipp"
#include "libnetwrk/asio/asio/detail/impl/posix_mutex.ipp"
#include "libnetwrk/asio/asio/detail/impl/posix_thread.ipp"
#include "libnetwrk/asio/asio/detail/impl/posix_tss_ptr.ipp"
#include "libnetwrk/asio/asio/detail/impl/reactive_descriptor_service.ipp"
#include "libnetwrk/asio/asio/detail/impl/reactive_serial_port_service.ipp"
#include "libnetwrk/asio/asio/detail/impl/reactive_socket_service_base.ipp"
#include "libnetwrk/asio/asio/detail/impl/resolver_service_base.ipp"
#include "libnetwrk/asio/asio/detail/impl/scheduler.ipp"
#include "libnetwrk/asio/asio/detail/impl/select_reactor.ipp"
#include "libnetwrk/asio/asio/detail/impl/service_registry.ipp"
#include "libnetwrk/asio/asio/detail/impl/signal_set_service.ipp"
#include "libnetwrk/asio/asio/detail/impl/socket_ops.ipp"
#include "libnetwrk/asio/asio/detail/impl/socket_select_interrupter.ipp"
#include "libnetwrk/asio/asio/detail/impl/strand_executor_service.ipp"
#include "libnetwrk/asio/asio/detail/impl/strand_service.ipp"
#include "libnetwrk/asio/asio/detail/impl/throw_error.ipp"
#include "libnetwrk/asio/asio/detail/impl/timer_queue_ptime.ipp"
#include "libnetwrk/asio/asio/detail/impl/timer_queue_set.ipp"
#include "libnetwrk/asio/asio/detail/impl/win_iocp_handle_service.ipp"
#include "libnetwrk/asio/asio/detail/impl/win_iocp_io_context.ipp"
#include "libnetwrk/asio/asio/detail/impl/win_iocp_serial_port_service.ipp"
#include "libnetwrk/asio/asio/detail/impl/win_iocp_socket_service_base.ipp"
#include "libnetwrk/asio/asio/detail/impl/win_event.ipp"
#include "libnetwrk/asio/asio/detail/impl/win_mutex.ipp"
#include "libnetwrk/asio/asio/detail/impl/win_object_handle_service.ipp"
#include "libnetwrk/asio/asio/detail/impl/win_static_mutex.ipp"
#include "libnetwrk/asio/asio/detail/impl/win_thread.ipp"
#include "libnetwrk/asio/asio/detail/impl/win_tss_ptr.ipp"
#include "libnetwrk/asio/asio/detail/impl/winrt_ssocket_service_base.ipp"
#include "libnetwrk/asio/asio/detail/impl/winrt_timer_scheduler.ipp"
#include "libnetwrk/asio/asio/detail/impl/winsock_init.ipp"
#include "libnetwrk/asio/asio/generic/detail/impl/endpoint.ipp"
#include "libnetwrk/asio/asio/ip/impl/address.ipp"
#include "libnetwrk/asio/asio/ip/impl/address_v4.ipp"
#include "libnetwrk/asio/asio/ip/impl/address_v6.ipp"
#include "libnetwrk/asio/asio/ip/impl/host_name.ipp"
#include "libnetwrk/asio/asio/ip/impl/network_v4.ipp"
#include "libnetwrk/asio/asio/ip/impl/network_v6.ipp"
#include "libnetwrk/asio/asio/ip/detail/impl/endpoint.ipp"
#include "libnetwrk/asio/asio/local/detail/impl/endpoint.ipp"

#endif // ASIO_IMPL_SRC_HPP
