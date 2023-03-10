#ifndef LIBNETWRK_NET_COMMON_EXCEPTIONS_LIBNETWRK_EXCEPTION_HPP
#define LIBNETWRK_NET_COMMON_EXCEPTIONS_LIBNETWRK_EXCEPTION_HPP

#include <exception>
#include <string>

namespace libnetwrk::net::common {
	class libnetwrk_exception : public std::exception {
		private:
			std::string m_message;

		public:
			libnetwrk_exception() : m_message("undefined") {}
			libnetwrk_exception(const char* msg) : m_message(msg) {}

			const char* msg() {
				return m_message.c_str();
			}
	};
}

#endif