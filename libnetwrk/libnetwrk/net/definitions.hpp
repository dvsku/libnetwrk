#ifndef LIBNETWRK_DEFINITIONS_HPP
#define LIBNETWRK_DEFINITIONS_HPP

#include <vector>
#include <string>
#include <memory>

#include "asio.hpp"

namespace libnetwrk {
	// Empty struct
	struct nothing {};

	typedef std::shared_ptr<asio::io_context> context_ptr;
	typedef std::shared_ptr<asio::ip::tcp::acceptor> acceptor_ptr;
}

#define BUFFER_		std::vector
#define BUFFER_U8	std::vector<uint8_t>
#define BUFFER_U16	std::vector<uint16_t>
#define BUFFER_U32	std::vector<uint32_t>
#define BUFFER_U64	std::vector<uint64_t>
#define BUFFER_STR	std::vector<std::string>

// use to suppress unreferenced local variable warnings
#define VAR_IGNORE(x) (void)x;

#endif