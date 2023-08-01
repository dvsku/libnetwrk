#ifndef LIBNETWRK_LOG_HPP
#define LIBNETWRK_LOG_HPP

#include <string>
#include <time.h>
#include <iostream>
#include <fstream>
#include <chrono>

#define FMT_HEADER_ONLY
#include "libnetwrk/fmt/format.h"
#include "libnetwrk/fmt/chrono.h"
#include "libnetwrk/fmt/color.h"

#include "libnetwrk/utilities/traits/non_copyable.hpp"
#include "libnetwrk/utilities/traits/non_moveable.hpp"

#if defined(_WIN32) || defined(WIN32)
	#include "windows.h"
	#define __LIBNETWRK_SNPRINTF(buffer, size, format, ...) _snprintf(buffer, size, format, __VA_ARGS__)
	#define __LIBNETWRK_LOCALTIME(time_t_addr, dst_addr) localtime_s(dst_addr, time_t_addr)
#else
	#define __LIBNETWRK_SNPRINTF(buffer, size, format, ...) snprintf(buffer, size, format, __VA_ARGS__)
	#define __LIBNETWRK_LOCALTIME(time_t_addr, dst_addr) localtime_r(time_t_addr, dst_addr)
#endif

///////////////////////////////////////////////////////////////////////////////
// MACROS
///////////////////////////////////////////////////////////////////////////////

#define LIBNETWRK_FORMAT(fmt, ...)				\
	fmt::format(fmt, ##__VA_ARGS__)

#define LIBNETWRK_INFO(name, fmt, ...)			\
	libnetwrk::log::log_message(libnetwrk::log_level::informational,	name, fmt, ##__VA_ARGS__)

#define LIBNETWRK_WARNING(name, fmt, ...)		\
	libnetwrk::log::log_message(libnetwrk::log_level::warning,			name, fmt, ##__VA_ARGS__)

#define LIBNETWRK_ERROR(name, fmt, ...)			\
	libnetwrk::log::log_message(libnetwrk::log_level::error,			name, fmt, ##__VA_ARGS__)

#define LIBNETWRK_DEBUG(name, fmt, ...)			\
	libnetwrk::log::log_message(libnetwrk::log_level::debug,			name, fmt, ##__VA_ARGS__)

#define LIBNETWRK_VERBOSE(name, fmt, ...)		\
	libnetwrk::log::log_message(libnetwrk::log_level::verbose,			name, fmt, ##__VA_ARGS__)

///////////////////////////////////////////////////////////////////////////////
// Only define these macros if they aren't already defined as to not
//	clash with existing log libs

#ifndef LOG_INFO
#define LOG_INFO(name, fmt, ...)				\
	libnetwrk::log::log_message(libnetwrk::log_level::informational,	name, fmt, ##__VA_ARGS__)
#endif

#ifndef LOG_WARN
#define LOG_WARN(name, fmt, ...)				\
	libnetwrk::log::log_message(libnetwrk::log_level::warning,			name, fmt, ##__VA_ARGS__)
#endif

#ifndef LOG_ERROR
#define LOG_ERROR(name, fmt, ...)				\
	libnetwrk::log::log_message(libnetwrk::log_level::error,			name, fmt, ##__VA_ARGS__)
#endif

#ifndef LOG_DEBUG
#define LOG_DEBUG(name, fmt, ...)				\
	libnetwrk::log::log_message(libnetwrk::log_level::debug,			name, fmt, ##__VA_ARGS__)
#endif

#ifndef LOG_VERB
#define LOG_VERB(name, fmt, ...)				\
	libnetwrk::log::log_message(libnetwrk::log_level::verbose,			name, fmt, ##__VA_ARGS__)
#endif

///////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION
///////////////////////////////////////////////////////////////////////////////

namespace libnetwrk {
	enum class log_level : unsigned char {
		none			= 0x00,		// does not log
		informational	= 0x01,		// logs only info
		warning			= 0x02,		// logs info and warnings
		error			= 0x03,		// logs info, warnings and errors
		debug			= 0x04,		// logs info, warnings and errors with additional details 
		verbose			= 0x05		// logs everything
	};
	
	class log : public non_copyable, public non_moveable {
	public:
		static void init(log_level level, bool log_to_console = true, bool log_to_file = false) {
			if (m_logger) return;
			m_logger = std::make_unique<logger>(level, log_to_console, log_to_file);

		#if defined(_WIN32) || defined(WIN32)
			// Enable colors in console (disabled by default)
			SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), 7);
		#endif
		}

		static void set_level(log_level level) {
			if (m_logger) 
				m_logger->level = level;
		}

		static void set_log_to_console(bool log_to_console) {
			if (m_logger) 
				m_logger->log_to_console = log_to_console;
		}

		static void set_log_to_file(bool log_to_file) {
			if (m_logger) 
				m_logger->log_to_file = log_to_file;
		}

		static void set_log_name(const std::string& name) {
			if (m_logger) 
				m_logger->log_name = name;
		}

		static void set_log_dir(const std::string& dir) {
			if (m_logger) 
				m_logger->log_dir = dir;
		}

		template <typename... Targs>
		static void log_message(log_level level, const std::string& name, fmt::string_view format, Targs&&... args) {
			if (m_logger)
				m_logger->log(level, name, format, args...);
		}

	private:
		class logger : public non_copyable, public non_moveable {
		public:
			log_level level			= log_level::error;
			bool log_to_console		= true;
			bool log_to_file		= true;
			std::string log_name	= "log";
			std::string log_dir		= "logs/";

		public:
			logger() = delete;
			logger(log_level _lev, bool _log_to_console, bool _log_to_file) 
				: level(_lev), log_to_console(_log_to_console), log_to_file(_log_to_file) 
			{
				auto timezone = std::chrono::current_zone();
				m_timezone_offset = timezone ? 
					timezone->get_info(std::chrono::system_clock::now()).offset : std::chrono::seconds(0);
			}

			template <typename... Targs>
			void log(log_level _lev, const std::string& _name, fmt::string_view _format, Targs&&... _args) {
				if (level < _lev) return;
				if (!log_to_console && !log_to_file) return;

				auto local		= std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now() + m_timezone_offset);
				auto prefix		= m_prefixes[(unsigned int)_lev - 1];
				auto formatted	= fmt::vformat(_format, fmt::make_format_args(std::forward<Targs>(_args)...));

				if (log_to_console) {
					std::string msg;

					if (_name != "")
						msg = fmt::format("[{:%X} {}] [{}] {}\n", local, fmt::styled(prefix.abbr, 
							fmt::fg(prefix.color)), fmt::styled(_name, fmt::fg(fmt::color::blue_violet)), formatted);
					else
						msg = fmt::format("[{:%X} {}] {}\n", local, fmt::styled(prefix.abbr,
							fmt::fg(prefix.color)), formatted);

					print_to_console(msg);
				}

				if (log_to_file) {
					std::string msg;

					if (_name != "")
						msg = fmt::format("[{:%X} {}] [{}] {}\n", local, prefix.abbr, _name, formatted);
					else
						msg = fmt::format("[{:%X} {}] {}\n", local, prefix.abbr, formatted);

					write_to_file(msg);
				}
			}

		private:
			struct prefix {
				const char* abbr;
				fmt::color color;
			};

			static inline const prefix m_prefixes[] = {
				{"INFO", fmt::color::white_smoke},
				{"WARN", fmt::color::yellow},
				{"ERRO", fmt::color::red},
				{"DEBG", fmt::color::white_smoke},
				{"VERB", fmt::color::dark_cyan}
			};

			std::chrono::seconds m_timezone_offset;

		private:
			void print_to_console(const std::string& str) {
				try {
					fmt::print(fmt::text_style(), str);
				}
				catch (...) {}
			}

			void write_to_file(const std::string& str) {
				try {
					char time_buffer[32];

					// Get current time
					time_t		now = time(0);
					struct tm	tstruct;

					__LIBNETWRK_LOCALTIME(&now, &tstruct);

					// Format time to time_buffer
					strftime(time_buffer, sizeof(time_buffer), "%d-%m-%Y", &tstruct);

					// Create file name
					std::string file_name = log_name + "_" + time_buffer + ".txt";

					std::ofstream out;
					out.open(file_name.c_str(), std::ios_base::app);
					if (out.is_open() && out.good())
						out << str;
				}
				catch (...) {}
			}
		};

		inline static std::unique_ptr<logger> m_logger = nullptr;
	};
}

#endif