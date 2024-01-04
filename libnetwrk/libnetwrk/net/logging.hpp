#pragma once

#ifdef LIBNETWRK_DISABLE_EMBEDDED_LOGGING
    #ifndef LIBNETWRK_INFO
        #define LIBNETWRK_INFO(name, fmt, ...)    do {} while(0)
    #endif
    #ifndef LIBNETWRK_WARNING
        #define LIBNETWRK_WARNING(name, fmt, ...) do {} while(0)
    #endif
    #ifndef LIBNETWRK_ERROR
        #define LIBNETWRK_ERROR(name, fmt, ...)   do {} while(0)
    #endif
    #ifndef LIBNETWRK_DEBUG
        #define LIBNETWRK_DEBUG(name, fmt, ...)   do {} while(0)
    #endif
    #ifndef LIBNETWRK_VERBOSE
        #define LIBNETWRK_VERBOSE(name, fmt, ...) do {} while(0)
    #endif
#else
    #include "libnetwrk/utilities/dvsku_log.hpp"

    #define LIBNETWRK_INFO(name, fmt, ...)          \
        dvsku::log::log_message(dvsku::log_level::informational, name, fmt, ##__VA_ARGS__)

    #define LIBNETWRK_WARNING(name, fmt, ...)       \
        dvsku::log::log_message(dvsku::log_level::warning,       name, fmt, ##__VA_ARGS__)

    #define LIBNETWRK_ERROR(name, fmt, ...)         \
        dvsku::log::log_message(dvsku::log_level::error,         name, fmt, ##__VA_ARGS__)

    #define LIBNETWRK_DEBUG(name, fmt, ...)         \
        dvsku::log::log_message(dvsku::log_level::debug,         name, fmt, ##__VA_ARGS__)

    #define LIBNETWRK_VERBOSE(name, fmt, ...)       \
        dvsku::log::log_message(dvsku::log_level::verbose,       name, fmt, ##__VA_ARGS__)
#endif