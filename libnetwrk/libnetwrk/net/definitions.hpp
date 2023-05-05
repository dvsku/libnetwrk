#ifndef LIBNETWRK_DEFINITIONS_HPP
#define LIBNETWRK_DEFINITIONS_HPP

#include <vector>
#include <string>
#include <memory>

#include "libnetwrk/asio/asio.hpp"
#include "libnetwrk/utilities/log.hpp"

namespace libnetwrk {
	// Empty struct
	struct nothing {};

	typedef std::shared_ptr<asio::io_context> context_ptr;
	typedef std::shared_ptr<asio::ip::tcp::acceptor> acceptor_ptr;
	typedef std::lock_guard<std::mutex> libnetwrk_guard;

	enum class endpoint : unsigned int {
		localhost,
		v4
	};

	enum class connection_owner : unsigned int {
		server, 
		client
	};
}

// use to suppress unreferenced local variable warnings
#define VAR_IGNORE(x) (void)x;

#endif