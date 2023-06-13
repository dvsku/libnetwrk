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

#if defined(_WIN32) || defined(WIN32)
	#define LIBNETWRK_SNPRINTF(buffer, size, format, ...) _snprintf(buffer, size, format, __VA_ARGS__)
	#define LIBNETWRK_LOCALTIME(time_t_addr, dst_addr) localtime_s(dst_addr, time_t_addr)
#else
	#define LIBNETWRK_SNPRINTF(buffer, size, format, ...) snprintf(buffer, size, format, __VA_ARGS__)
	#define LIBNETWRK_LOCALTIME(time_t_addr, dst_addr) localtime_r(time_t_addr, dst_addr)
#endif

#ifndef LIBNETWRK_DEFAULT_LOG_BUFFER_SIZE
	#define LIBNETWRK_DEFAULT_LOG_BUFFER_SIZE 512U
#endif

#define LIBNETWRK_FORMAT(fmt, ...)				\
	fmt::format(fmt, ##__VA_ARGS__)

#define LIBNETWRK_INFO(fmt, ...)				\
	libnetwrk::log::instance().log_message(libnetwrk::log_severity::informational, "", fmt, ##__VA_ARGS__)

#define LIBNETWRK_INFO_A(name, fmt, ...)		\
	libnetwrk::log::instance().log_message(libnetwrk::log_severity::informational, name, fmt, ##__VA_ARGS__)

#define LIBNETWRK_WARNING(fmt, ...)				\
	libnetwrk::log::instance().log_message(libnetwrk::log_severity::warning, "", fmt, ##__VA_ARGS__)

#define LIBNETWRK_WARNING_A(name, fmt, ...)		\
	libnetwrk::log::instance().log_message(libnetwrk::log_severity::warning, name, fmt, ##__VA_ARGS__)

#define LIBNETWRK_ERROR(fmt, ...)				\
	libnetwrk::log::instance().log_message(libnetwrk::log_severity::error, "", fmt, ##__VA_ARGS__)

#define LIBNETWRK_ERROR_A(name, fmt, ...)		\
	libnetwrk::log::instance().log_message(libnetwrk::log_severity::error, name, fmt, ##__VA_ARGS__)

#define LIBNETWRK_VERBOSE(fmt, ...)				\
	libnetwrk::log::instance().log_message(libnetwrk::log_severity::verbose, "", fmt, ##__VA_ARGS__)

#define LIBNETWRK_VERBOSE_A(name, fmt, ...)		\
	libnetwrk::log::instance().log_message(libnetwrk::log_severity::verbose, name, fmt, ##__VA_ARGS__)

namespace libnetwrk {
	enum class log_severity : unsigned char {
		none			= 0x00,		// does not log
		informational	= 0x01,		// logs only info
		warning			= 0x02,		// logs info and warnings
		error			= 0x03,		// logs info, warnings and errors
		verbose			= 0x04		// logs info, warnings and errors with additional details
	};

	struct log_settings {
		log_severity m_severity = log_severity::error;

		// Disables logging to console if set to false, 
		//	logs to console depending on severity if set to true.
		// Set to true by default.
		bool m_log_to_console = true;

		// Disables logging to file if set to false,
		//	logs to file depending on severity if set to true.
		// Set to false by default.
		bool m_log_to_file = false;

		// Prefix used when creating a log file.
		// Structure: 
		//	name_prefix + _ + timestamp + .txt
		// Set to 'log' by default.
		std::string m_name_prefix = "log";
	};
	
	class log {
		protected:
			log_settings m_settings{};

			struct prefix {
				const char* m_abbr;
				fmt::color m_color;
			};

			static inline const prefix m_prefixes[] = {
				{"INFO", fmt::color::white},
				{"WARN", fmt::color::yellow},
				{"ERRO", fmt::color::red},
				{"VERB", fmt::color::dark_cyan}
			};

		private:
			log() {}

		public:
			log(log const&) = delete;
			void operator=(log const&) = delete;

			static log& instance() {
				static log instance;
				return instance;
			}

			void set_settings(const log_settings settings) {
				m_settings = settings;
			}

			template <typename... Targs>
			void log_message(log_severity severity, const std::string& name, fmt::format_string<Targs...> format, Targs&&... args) {
				auto time = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
				auto prefix = m_prefixes[(unsigned int)severity - 1];
				
				auto formatted = fmt::format(format, std::forward<Targs>(args)...);

				if (m_settings.m_log_to_console) {
					std::string msg;

					if (name != "")
						msg = fmt::format("[{:%X} {}] [{}] {}\n",
							time,
							fmt::styled(prefix.m_abbr, fmt::fg(prefix.m_color)),
							fmt::styled(name, fmt::fg(fmt::color::blue_violet)),
							formatted);
					else
						msg = fmt::format("[{:%X} {}] {}\n",
							time,
							fmt::styled(prefix.m_abbr, fmt::fg(prefix.m_color)),
							formatted);

					print(msg);
				}

				if (m_settings.m_log_to_file) {
					std::string msg;

					if (name != "")
						msg = fmt::format("[{:%X} {}] [{}] {}\n",
							time,
							prefix.m_abbr,
							name,
							formatted);
					else
						msg = fmt::format("[{:%X} {}] {}\n",
							time,
							prefix.m_abbr,
							formatted);

					write(msg);
				}
			}

		protected:
			void print(const std::string& str) {
				fmt::print(fmt::text_style(), str);
			}

			void write(const std::string& str) {
				char time_buffer[32];

				// Get current time
				time_t		now = time(0);
				struct tm	tstruct;

				LIBNETWRK_LOCALTIME(&now, &tstruct);

				// Format time to time_buffer
				strftime(time_buffer, sizeof(time_buffer), "%d-%m-%Y", &tstruct);

				// Create file name
				std::string file_name = m_settings.m_name_prefix + "_" + time_buffer + ".txt";

				std::ofstream out;
				out.open(file_name.c_str(), std::ios_base::app);
				out << str;
			}
	};
}

#endif