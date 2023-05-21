#ifndef LIBNETWRK_LOG_HPP
#define LIBNETWRK_LOG_HPP

#include <string>
#include <time.h>
#include <stdarg.h>
#include <iostream>
#include <fstream>

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

#define LIBNETWRK_INFO(fmt, ...)				\
	libnetwrk::log::instance().log_message(libnetwrk::log_severity::informational, nullptr, fmt, ##__VA_ARGS__)

#define LIBNETWRK_INFO_A(name, fmt, ...)		\
	libnetwrk::log::instance().log_message(libnetwrk::log_severity::informational, name, fmt, ##__VA_ARGS__)

#define LIBNETWRK_WARNING(fmt, ...)				\
	libnetwrk::log::instance().log_message(libnetwrk::log_severity::warning, nullptr, fmt, ##__VA_ARGS__)

#define LIBNETWRK_WARNING_A(name, fmt, ...)		\
	libnetwrk::log::instance().log_message(libnetwrk::log_severity::warning, name, fmt, ##__VA_ARGS__)

#define LIBNETWRK_ERROR(fmt, ...)				\
	libnetwrk::log::instance().log_message(libnetwrk::log_severity::error, nullptr, fmt, ##__VA_ARGS__)

#define LIBNETWRK_ERROR_A(name, fmt, ...)		\
	libnetwrk::log::instance().log_message(libnetwrk::log_severity::error, name, fmt, ##__VA_ARGS__)

#define LIBNETWRK_VERBOSE(fmt, ...)				\
	libnetwrk::log::instance().log_message(libnetwrk::log_severity::verbose, nullptr, fmt, ##__VA_ARGS__)

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

			static inline const char* m_prefixes[] = {
				"INFO", "WARN", "ERRO", "VERB"
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

			void log_message(log_severity severity, const char* name, const char* fmt, ...) {
				// Get args list
				va_list args;
				va_start(args, fmt);

				_log_message(severity, name, fmt, args);

				va_end(args);
			}

			void log_message(log_severity severity, const std::string& name, const char* fmt, ...) {
				// Get args list
				va_list args;
				va_start(args, fmt);

				_log_message(severity, name.c_str(), fmt, args);

				va_end(args);
			}

		protected:
			void _log_message(const std::string& str) {
				if (m_settings.m_log_to_console)
					print(str);

				if (m_settings.m_log_to_file)
					write(str);
			}

			void _log_message(log_severity severity, const char* name, const char* fmt, va_list args) {
				if (m_settings.m_severity < severity) return;
				if (!m_settings.m_log_to_console && !m_settings.m_log_to_file) return;

				std::string str;

				// Format message
				format(str, LIBNETWRK_DEFAULT_LOG_BUFFER_SIZE, severity, name, fmt, args);

				_log_message(str);
			}

			void format(std::string& str, size_t size, log_severity severity, 
				const char* name, const char* fmt, va_list args) 
			{
				// Make sure buffer is at least 512 bytes
				if (size < 512U) size = 512U;

				str.resize(size);

				char time_buffer[32];

				// Get current time
				time_t		now = time(0);
				struct tm	tstruct;

				LIBNETWRK_LOCALTIME(&now, &tstruct);

				// Format time to time_buffer
				strftime(time_buffer, sizeof(time_buffer), "%X", &tstruct);

				// Copy time and prefix to str
				int i = 0;

				if(name == nullptr)
					i = LIBNETWRK_SNPRINTF(str.data(), size, "[%s %s] ",
						time_buffer, m_prefixes[(unsigned int)severity - 1]);
				else
					i = LIBNETWRK_SNPRINTF(str.data(), size, "[%s %s] [%s] ",
						time_buffer, m_prefixes[(unsigned int)severity - 1], name);
				
				// __SNPRINTF failed to encode time_buffer and prefix
				if (i == -1)
					return log_message(log_severity::warning, nullptr,
						"failed to log message. encoding error occurred.");
				
				// Buffer not large enough
				if ((size_t)i >= size)
					return format(str, size * 2, severity, name, fmt, args);

				// Format fmt to str
				i = vsnprintf(str.data() + i, size - i, fmt, args);

				// __SNPRINTF failed to encode args
				if (i == -1)
					return log_message(log_severity::warning, nullptr,
						"failed to log message. encoding error occurred.");

				// Buffer not large enough
				if ((size_t)i >= size)
					return format(str, size * 2, severity, name, fmt, args);

				str.erase(std::find(str.begin(), str.end(), '\0'), str.end());
				str.append(1, '\n');
			}

			void print(const std::string& str) {
				std::cout << str;
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