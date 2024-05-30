#pragma once

#ifdef _WIN32
    #include <intrin.h>
    #define LIBNETWRK_SWAP_ENDIAN_16(x) _byteswap_ushort(x)
    #define LIBNETWRK_SWAP_ENDIAN_32(x) _byteswap_ulong(x)
    #define LIBNETWRK_SWAP_ENDIAN_64(x) _byteswap_uint64(x)
#elif defined(__linux__)
    #define LIBNETWRK_SWAP_ENDIAN_16(x) __builtin_bswap16(x)
    #define LIBNETWRK_SWAP_ENDIAN_32(x) __builtin_bswap32(x)
    #define LIBNETWRK_SWAP_ENDIAN_64(x) __builtin_bswap64(x)
#endif

#include <cstdint>
#include <type_traits>

namespace {
    constexpr bool is_system_little_endian() {
        int32_t x = 0xaabbccdd;
        uint8_t y = static_cast<uint8_t>(x);
        return (y == 0xdd);
    }

    static inline void swap_uint16(uint16_t* val) {
        *val = LIBNETWRK_SWAP_ENDIAN_16(*val);
    }

    static inline void swap_int16(int16_t* val) {
        swap_uint16((uint16_t*)val);
    }

    static inline void swap_uint32(uint32_t* val) {
        *val = LIBNETWRK_SWAP_ENDIAN_32(*val);
    }

    static inline void swap_int32(int32_t* val) {
        swap_uint32((uint32_t*)val);
    }

    static inline void swap_uint64(uint64_t* val) {
        *val = LIBNETWRK_SWAP_ENDIAN_64(*val);
    }

    static inline void swap_int64(int64_t* val) {
        swap_uint64((uint64_t*)val);
    }

    static inline void swap_float(float* val) {
        swap_uint32((uint32_t*)val);
    }

    static inline void swap_double(double* val) {
        swap_uint64((uint64_t*)val);
    }
}

namespace libnetwrk::serialize::internal {
    template <typename Type>
    constexpr auto byte_swap(Type& value) {
        if constexpr (std::same_as<Type, int16_t>) {
            swap_int16(&value);
        }
        else if constexpr (std::same_as<Type, uint16_t>) {
            swap_uint16(&value);
        }
        else if constexpr (std::same_as<Type, int32_t>) {
            swap_int32(&value);
        }
        else if constexpr (std::same_as<Type, uint32_t>) {
            swap_uint32(&value);
        }
        else if constexpr (std::same_as<Type, int64_t>) {
            swap_int64(&value);
        }
        else if constexpr (std::same_as<Type, uint64_t>) {
            swap_uint64(&value);
        }
        else if constexpr (std::same_as<Type, float>) {
            swap_float(&value);
        }
        else if constexpr (std::same_as<Type, double>) {
            swap_double(&value);
        }
    }

    template <typename Type>
    constexpr auto to_little_endian(Type& value) {
        if constexpr (is_system_little_endian())
            return;

        return byte_swap(value);
    }
}
