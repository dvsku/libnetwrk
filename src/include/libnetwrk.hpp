#ifndef LIBNETWRK_HPP
#define LIBNETWRK_HPP

#include "net/message.hpp"

#if defined(LIBNETWRK_SERVER)
	#if defined(LIBNETWRK_TCP)
		#include "net/tcp/tcp_connection.hpp"
		#include "net/tcp/tcp_server.hpp"
	#elif defined(LIBNETWRK_UDP)

	#else
		#include "net/tcp/tcp_connection.hpp"
		#include "net/tcp/tcp_server.hpp"
	#endif
#elif defined(LIBNETWRK_CLIENT)
	#if defined(LIBNETWRK_TCP)
		#include "net/tcp/tcp_connection.hpp"
		#include "net/tcp/tcp_client.hpp"
	#elif defined(LIBNETWRK_UDP)
	
	#else
		#include "net/tcp/tcp_connection.hpp"
		#include "net/tcp/tcp_client.hpp"
	#endif
#else
	#if defined(LIBNETWRK_TCP)
		#include "net/tcp/tcp_connection.hpp"
		#include "net/tcp/tcp_server.hpp"
		#include "net/tcp/tcp_client.hpp"
	#elif defined(LIBNETWRK_UDP)

	#else
		#include "net/tcp/tcp_connection.hpp"
		#include "net/tcp/tcp_server.hpp"
		#include "net/tcp/tcp_client.hpp"
	#endif
#endif

#endif