#pragma once

#ifdef _WIN32
    #include <windows.h>
#else
    #include <ctime>
#endif

#include <cstdint>

namespace libnetwrk {
    inline uint64_t get_milliseconds_timestamp() {
    #ifdef _WIN32
        SYSTEMTIME      st{};
        FILETIME        ft{};
        ULARGE_INTEGER uli{};

        GetSystemTime(&st);
        SystemTimeToFileTime(&st, &ft);

        uli.LowPart  = ft.dwLowDateTime;
        uli.HighPart = ft.dwHighDateTime;

        return (uli.QuadPart / 10000) - 11644473600000ULL;
    #else
        timespec ts{};
        clock_gettime(CLOCK_REALTIME, &ts);

        return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
    #endif
    }
}
