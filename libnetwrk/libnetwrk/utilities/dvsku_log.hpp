#pragma once

#define FMT_HEADER_ONLY
#include "lib/fmt/chrono.h"
#include "lib/fmt/color.h"
#include "lib/fmt/format.h"

#if defined(_WIN32) || defined(WIN32)
    #include <windows.h>
#endif

#include <string>
#include <time.h>
#include <iostream>
#include <fstream>
#include <chrono>

#ifndef DV_LOG_INFO
    #define DV_LOG_INFO(name, fmt, ...)     \
        dvsku::log::log_message(dvsku::log_level::informational, name, fmt, ##__VA_ARGS__)
#endif

#ifndef DV_LOG_WARN
    #define DV_LOG_WARN(name, fmt, ...)     \
        dvsku::log::log_message(dvsku::log_level::warning,       name, fmt, ##__VA_ARGS__)
#endif

#ifndef DV_LOG_ERRO
    #define DV_LOG_ERRO(name, fmt, ...)     \
        dvsku::log::log_message(dvsku::log_level::error,         name, fmt, ##__VA_ARGS__)
#endif

#ifndef DV_LOG_DEBG
    #define DV_LOG_DEBG(name, fmt, ...)     \
        dvsku::log::log_message(dvsku::log_level::debug,         name, fmt, ##__VA_ARGS__)
#endif

#ifndef DV_LOG_VERB
    #define DV_LOG_VERB(name, fmt, ...)     \
        dvsku::log::log_message(dvsku::log_level::verbose,       name, fmt, ##__VA_ARGS__)
#endif

///////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION
///////////////////////////////////////////////////////////////////////////////

namespace dvsku {
    enum class log_level : unsigned char {
        none          = 0x00,        // does not log
        informational = 0x01,        // logs only info
        warning       = 0x02,        // logs info and warnings
        error         = 0x03,        // logs info, warnings and errors
        debug         = 0x04,        // logs info, warnings, errors and debug
        verbose       = 0x05         // logs everything
    };

    struct log_settings {
        log_level level      = log_level::error;
        bool log_to_console  = true;
        bool log_to_file     = true;
        std::string log_name = "log";
    };
    
    class log {
    public:
        static void init(log_level level, bool log_to_console = true, bool log_to_file = false) {
            if (m_log_impl) return;
            m_log_impl = std::make_unique<log_impl>(level, log_to_console, log_to_file);

        #if defined(_WIN32) || defined(WIN32)
            // Enable colors in console (disabled by default)
            SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), 7);
        #endif
        }

        static log_settings* get_settings() {
            if (m_log_impl) return nullptr;

            return &m_log_impl->settings;
        }

        template <typename... Targs>
        static void log_message(log_level level, const std::string& name, fmt::string_view format, Targs&&... args) {
            if (!m_log_impl) return;

            m_log_impl->log(level, name, format, args...);
        }

    private:
        class log_impl {
        public:
            log_settings settings;

        public:
            log_impl()                = delete;
            log_impl(const log_impl&) = delete;
            log_impl(log_impl&&)      = delete;

            log_impl(log_level level, bool log_to_console, bool log_to_file) {
                settings.level          = level;
                settings.log_to_console = log_to_console;
                settings.log_to_file    = log_to_file;
            }

            log_impl& operator=(const log_impl&) = delete;
            log_impl& operator=(log_impl&&)      = delete;

        public:
            template <typename... Targs>
            void log(log_level level, const std::string& name, fmt::string_view format, Targs&&... args) {
                if (settings.level < level)                            return;
                if (!settings.log_to_console && !settings.log_to_file) return;

                auto local_time = fmt::localtime(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
                auto prefix     = m_prefixes[(unsigned char)level - 1];
                auto formatted  = fmt::vformat(format, fmt::make_format_args(std::forward<Targs>(args)...));

                if (settings.log_to_console) {
                    std::string msg;

                    if (name != "") {
                        msg = fmt::format("[{:%X} {}] [{}] {}\n", local_time, fmt::styled(prefix.abbr, fmt::fg(prefix.color)),
                            fmt::styled(name, fmt::fg(fmt::color::blue_violet)), formatted);
                    }
                    else {
                        msg = fmt::format("[{:%X} {}] {}\n", local_time, fmt::styled(prefix.abbr, fmt::fg(prefix.color)), formatted);
                    }

                    print_to_console(msg);
                }

                if (settings.log_to_file) {
                    std::string msg;

                    if (name != "") {
                        msg = fmt::format("[{:%X} {}] [{}] {}\n", local_time, prefix.abbr, name, formatted);
                    }
                    else {
                        msg = fmt::format("[{:%X} {}] {}\n", local_time, prefix.abbr, formatted);
                    }

                    write_to_file(msg);
                }
            }

        private:
            struct prefix {
                const char* abbr;
                fmt::color  color;
            };

            static inline const prefix m_prefixes[] = {
                {"INFO", fmt::color::white_smoke},
                {"WARN", fmt::color::yellow},
                {"ERRO", fmt::color::red},
                {"DEBG", fmt::color::white_smoke},
                {"VERB", fmt::color::dark_cyan}
            };

        private:
            void print_to_console(const std::string& str) {
                try {
                #if (defined(_WIN32) || defined(_WIN64)) && defined(LIBNETWRK_DLL)
                    auto handle = GetStdHandle(STD_OUTPUT_HANDLE);
                    if(handle)
                        WriteConsole(handle, str.c_str(), str.length(), NULL, NULL);
                #else
                    fmt::print(fmt::text_style(), str);
                #endif    
                }
                catch (...) {}
            }

            void write_to_file(const std::string& str) {
                try {
                    auto local = fmt::localtime(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
                    std::string file_name = fmt::format("{}_{:%d-%m-%Y}.txt", settings.log_name, local);

                    std::ofstream out;
                    out.open(file_name.c_str(), std::ios_base::app);
                    if (out.is_open() && out.good())
                        out << str;
                }
                catch (...) {}
            }
        };

        inline static std::unique_ptr<log_impl> m_log_impl = nullptr;
    };
}
