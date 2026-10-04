cmake_minimum_required(VERSION 3.24)

project(
    block_compressor
    VERSION 1.0.0
    LANGUAGES CXX
)

# ---------------------------------------------------------------------------
# Options
# ---------------------------------------------------------------------------

option(
    BLOCK_COMPRESSOR_ENABLE_ZSTD
    "Enable headers that depend on Zstd"
    OFF
)

option(
    BLOCK_COMPRESSOR_ENABLE_CDS
    "Enable headers that depend on cds"
    OFF
)

# ---------------------------------------------------------------------------
# Core header-only library
# ---------------------------------------------------------------------------

add_library(block_compressor INTERFACE)
add_library(block_compressor::core ALIAS block_compressor)

target_include_directories(
    block_compressor
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(
    block_compressor
    INTERFACE
        cxx_std_17
)

target_sources(
    block_compressor
    INTERFACE
    FILE_SET HEADERS
    BASE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        include/block_compressor/block_compressor.hpp
        include/block_compressor/block_decompressor.hpp
        include/block_compressor/compressor.hpp
        include/block_compressor/config.hpp
        include/block_compressor/config_io.hpp
        include/block_compressor/decompressor.hpp
        include/block_compressor/error.hpp
        include/block_compressor/int_container.hpp
        include/block_compressor/stream.hpp
        include/block_compressor/utils.hpp
)

# ---------------------------------------------------------------------------
# External dependencies
# ---------------------------------------------------------------------------

add_subdirectory(external)

# ---------------------------------------------------------------------------
# Optional Zstd-facing interface
# ---------------------------------------------------------------------------

if(BLOCK_COMPRESSOR_ENABLE_ZSTD)
    add_library(block_compressor_zstd INTERFACE)
    add_library(block_compressor::zstd ALIAS block_compressor_zstd)

    target_link_libraries(
        block_compressor_zstd
        INTERFACE
            block_compressor
            external::zstd
    )

    target_sources(
        block_compressor_zstd
        INTERFACE
        FILE_SET HEADERS
        BASE_DIRS
            ${CMAKE_CURRENT_SOURCE_DIR}/include
        FILES
            include/block_compressor/compressor_zstd.hpp
            include/block_compressor/decompressor_zstd.hpp
            include/block_compressor/config_zstd.hpp
    )
endif()

# ---------------------------------------------------------------------------
# Optional cds-facing interface
# ---------------------------------------------------------------------------

if(BLOCK_COMPRESSOR_ENABLE_CDS)
    add_library(block_compressor_cds INTERFACE)
    add_library(block_compressor::cds ALIAS block_compressor_cds)

    target_link_libraries(
        block_compressor_cds
        INTERFACE
            block_compressor
            external::cds
    )

    # tlemane/cds currently requires C++23.
    target_compile_features(
        block_compressor_cds
        INTERFACE
            cxx_std_23
    )

    target_sources(
        block_compressor_cds
        INTERFACE
        FILE_SET HEADERS
        BASE_DIRS
            ${CMAKE_CURRENT_SOURCE_DIR}/include
        FILES
            include/block_compressor/int_container_ef.hpp
    )
endif()