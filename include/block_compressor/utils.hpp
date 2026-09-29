#ifndef BLOCK_COMPRESSOR_UTILS_H
#define BLOCK_COMPRESSOR_UTILS_H

#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <istream>
#include <ostream>
#include <string>
#include <memory>

#include <block_compressor/error.hpp>

namespace block_compressor::utils
{
    template <typename T>
    constexpr T ceil_div(T x, T y)
    {
        if (y == 0)
            throw block_compressor_error("utils", "ceil_div", "Attempted a division by 0");

        return (x + y - T{1}) / y;
    }

    template <typename T>
    constexpr T bits_to_bytes(T size)
    {
        return ceil_div(size, T{8});
    }

    template <typename T>
    constexpr T ceil_to_multiple(T x, T y)
    {
        return ceil_div<T>(x, y) * y;
    }

    template <typename T>
    constexpr T nearest_multiple(T x, T y)
    {
        if (y == 0)
            throw block_compressor_error("utils", "nearest_multiple", "x \% 0 is meaningless");

        T r = x % y;
        return (r < (y + T{1}) / T{2}) ? (x - r) : (x + (y - r));
    }
}

#endif