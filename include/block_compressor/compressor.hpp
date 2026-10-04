#ifndef BLOCK_COMPRESSOR_COMPRESSOR_H
#define BLOCK_COMPRESSOR_COMPRESSOR_H

#include <cstdint>
#include <cstring>

#include <block_compressor/error.hpp>

namespace block_compressor
{
    class Compressor
    {
    public:
        Compressor() = default;
        virtual ~Compressor() = default;
        virtual std::size_t compress(const char* input, char* output, std::size_t input_size, std::size_t output_size) = 0;
        virtual std::size_t compression_upper_bound(std::size_t size) = 0;
    };

    class CompressorIdentity : public Compressor
    {
    public:
        CompressorIdentity() = default;
        ~CompressorIdentity() = default;

        std::size_t compress(const char* input, char* output, std::size_t input_size, std::size_t output_size) override
        {
            if(output_size < input_size)
                throw block_compressor_error("CompressorIdentity", "compress", "Output size is not big enough");

            std::memcpy(output, input, input_size);
            return input_size;
        }

        std::size_t compression_upper_bound(std::size_t size) override
        {
            return size;
        }
    };
}
#endif
