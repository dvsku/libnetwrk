#pragma once

#include <libutil.hpp>

#define LIBNETWRK_INFO(component, frmt, ...)        \
    libutil::log::log_message(libutil::log::level::informational, component, frmt, ##__VA_ARGS__)

#define LIBNETWRK_WARNING(component, frmt, ...)     \
    libutil::log::log_message(libutil::log::level::warning,       component, frmt, ##__VA_ARGS__)

#define LIBNETWRK_ERROR(component, frmt, ...)       \
    libutil::log::log_message(libutil::log::level::error,         component, frmt, ##__VA_ARGS__)

#define LIBNETWRK_VERBOSE(component, frmt, ...)     \
    libutil::log::log_message(libutil::log::level::verbose,       component, frmt, ##__VA_ARGS__)

#define LIBNETWRK_DEBUG(component, frmt, ...)       \
    libutil::log::log_message(libutil::log::level::debug,         component, frmt, ##__VA_ARGS__)

#include <libnetwrk.hpp>
