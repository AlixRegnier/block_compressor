#ifndef BLOCK_COMPRESSOR_DECOMPRESSOR_H
#define BLOCK_COMPRESSOR_DECOMPRESSOR_H

#include <cstdint>
#include <cstring>

#include <block_compressor/error.hpp>

namespace block_compressor
{
    class Decompressor
    {
    public:
        Decompressor() = default;
        virtual ~Decompressor() = default;
        virtual std::size_t decompress(const char* input, char* output, std::size_t input_size, std::size_t output_size) = 0;
    };

    class DecompressorIdentity : public Decompressor
    {
    public:
        DecompressorIdentity() = default;
        ~DecompressorIdentity() = default;

        std::size_t decompress(const char* input, char* output, std::size_t input_size, std::size_t output_size) override
        {
            if(output_size < input_size)
                throw block_compressor_error("DecompressorIdentity", "decompress", "Output size is not big enough"); 

            std::memcpy(output, input, input_size);
            return input_size;
        }
    };
}
#endif
