#ifndef BLOCK_COMPRESSOR_DECOMPRESSOR_ZSTD_H
#define BLOCK_COMPRESSOR_DECOMPRESSOR_ZSTD_H

#include <zstd.h>

#include <block_compressor/decompressor.hpp>

namespace block_compressor
{
    class DecompressorZstd : public Decompressor
    {
    private:
        ZSTD_DCtx* context;
    public:
        DecompressorZstd()
        {
            context = ZSTD_createDCtx();
        }

        ~DecompressorZstd()
        {
            ZSTD_freeDCtx(context);
        }

        std::size_t decompress(const char* input, char* output, std::size_t input_size, std::size_t output_size) override
        {
            return ZSTD_decompressDCtx(context, output, output_size, input, input_size);
        }
    };
}

#endif
