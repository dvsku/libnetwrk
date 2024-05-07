#pragma once

#include <dv_utilities.hpp>

#define LIBNETWRK_INFO(component, frmt, ...)        \
    DV_LOG_INFO(component, frmt, __VA_ARGS__)

#define LIBNETWRK_WARNING(component, frmt, ...)     \
    DV_LOG_WARN(component, frmt, __VA_ARGS__)

#define LIBNETWRK_ERROR(component, frmt, ...)       \
    DV_LOG_ERRO(component, frmt, __VA_ARGS__)

#define LIBNETWRK_DEBUG(component, frmt, ...)       \
    DV_LOG_DEBG(component, frmt, __VA_ARGS__)

#define LIBNETWRK_VERBOSE(component, frmt, ...)     \
    DV_LOG_VERB(component, frmt, __VA_ARGS__)

#include <libnetwrk.hpp>
