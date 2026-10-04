if(BLOCK_COMPRESSOR_ENABLE_ZSTD)
    add_subdirectory(zstd)
endif()

if(BLOCK_COMPRESSOR_ENABLE_CDS)
    add_subdirectory(cds)
endif()
