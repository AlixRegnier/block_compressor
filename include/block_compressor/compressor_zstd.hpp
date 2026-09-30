#ifndef BLOCK_COMPRESSOR_COMPRESSOR_ZSTD_H
#define BLOCK_COMPRESSOR_COMPRESSOR_ZSTD_H

#include <zstd.h>

#include <block_compressor/compressor.hpp>
#include <block_compressor/config_zstd.hpp>

namespace block_compressor
{
    class CompressorZstd : public Compressor
    {
    private:
        ZSTD_CCtx* context;
    public:
        CompressorZstd(std::int64_t preset = ConfigZstd::default_preset, std::uint64_t wlog = ConfigZstd::default_wlog)
        {
            context = ZSTD_createCCtx();
            ZSTD_CCtx_setParameter(context, ZSTD_c_compressionLevel, preset);
            ZSTD_CCtx_setParameter(context, ZSTD_c_windowLog, wlog);
        }

        ~CompressorZstd()
        {
            ZSTD_freeCCtx(context);
        }

        std::size_t compress(const char* input, char* output, std::size_t input_size, std::size_t output_size) override
        {
            return ZSTD_compress2(context, output, output_size, input, input_size);
        }

        std::size_t compression_upper_bound(std::size_t size) override
        {
            return ZSTD_compressBound(size);
        }
    };
}

#endif
