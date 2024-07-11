#pragma once

#ifdef LIBNETWRK_DISABLE_LOGGING
    #ifdef LIBNETWRK_INFO
        #undef LIBNETWRK_INFO
    #endif
    #define LIBNETWRK_INFO(component, frmt, ...)    do {} while(0)

    #ifdef LIBNETWRK_WARNING
        #undef LIBNETWRK_WARNING
    #endif
    #define LIBNETWRK_WARNING(component, frmt, ...) do {} while(0)

    #ifdef LIBNETWRK_ERROR
        #undef LIBNETWRK_ERROR
    #endif
    #define LIBNETWRK_ERROR(component, frmt, ...)   do {} while(0)

    #ifdef LIBNETWRK_VERBOSE
        #undef LIBNETWRK_VERBOSE
    #endif
    #define LIBNETWRK_VERBOSE(component, frmt, ...) do {} while(0)

    #ifdef LIBNETWRK_DEBUG
        #undef LIBNETWRK_DEBUG
    #endif
    #define LIBNETWRK_DEBUG(component, frmt, ...)   do {} while(0)

    #ifdef LIBNETWRK_ENABLE_FILE_LOG
        #undef LIBNETWRK_ENABLE_FILE_LOG
    #endif
    #define LIBNETWRK_ENABLE_FILE_LOG()   do {} while(0)

    #ifdef LIBNETWRK_DISABLE_FILE_LOG
        #undef LIBNETWRK_DISABLE_FILE_LOG
    #endif
    #define LIBNETWRK_DISABLE_FILE_LOG()  do {} while(0)
#else
    #ifndef LIBNETWRK_INFO
        #define LIBNETWRK_INFO(component, frmt, ...)    do {} while(0)
    #endif

    #ifndef LIBNETWRK_WARNING
        #define LIBNETWRK_WARNING(component, frmt, ...) do {} while(0)
    #endif

    #ifndef LIBNETWRK_ERROR
        #define LIBNETWRK_ERROR(component, frmt, ...)   do {} while(0)
    #endif

    #ifndef LIBNETWRK_VERBOSE
        #define LIBNETWRK_VERBOSE(component, frmt, ...) do {} while(0)
    #endif

    #ifndef LIBNETWRK_DEBUG
        #define LIBNETWRK_DEBUG(component, frmt, ...)   do {} while(0)
    #endif

    #ifndef LIBNETWRK_ENABLE_FILE_LOG
        #define LIBNETWRK_ENABLE_FILE_LOG()  do {} while(0)
    #endif

    #ifndef LIBNETWRK_DISABLE_FILE_LOG
        #define LIBNETWRK_DISABLE_FILE_LOG() do {} while(0)
    #endif
#endif