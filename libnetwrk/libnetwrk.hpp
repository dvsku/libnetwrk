#ifndef LIBNETWRK_HPP
#define LIBNETWRK_HPP

#include "libnetwrk/net/message.hpp"
#include "libnetwrk/net/common/containers/buffer.hpp"

#if defined(LIBNETWRK_SERVER)
	#if defined(LIBNETWRK_TCP)
		#include "libnetwrk/net/tcp/tcp_connection.hpp"
		#include "libnetwrk/net/tcp/tcp_server.hpp"
	#elif defined(LIBNETWRK_UDP)

	#else
		#include "libnetwrk/net/tcp/tcp_connection.hpp"
		#include "libnetwrk/net/tcp/tcp_server.hpp"
	#endif
#elif defined(LIBNETWRK_CLIENT)
	#if defined(LIBNETWRK_TCP)
		#include "libnetwrk/net/tcp/tcp_connection.hpp"
		#include "libnetwrk/net/tcp/tcp_client.hpp"
	#elif defined(LIBNETWRK_UDP)
	
	#else
		#include "libnetwrk/net/tcp/tcp_connection.hpp"
		#include "libnetwrk/net/tcp/tcp_client.hpp"
	#endif
#else
	#if defined(LIBNETWRK_TCP)
		#include "libnetwrk/net/tcp/tcp_connection.hpp"
		#include "libnetwrk/net/tcp/tcp_server.hpp"
		#include "libnetwrk/net/tcp/tcp_client.hpp"
	#elif defined(LIBNETWRK_UDP)

	#else
		#include "libnetwrk/net/tcp/tcp_connection.hpp"
		#include "libnetwrk/net/tcp/tcp_server.hpp"
		#include "libnetwrk/net/tcp/tcp_client.hpp"
	#endif
#endif

#endif