include(ExternalProject)

# ---------------------------------------------------------------------------
# Paths
# ---------------------------------------------------------------------------

set(
    BLOCK_COMPRESSOR_ZSTD_PREFIX
    "${CMAKE_CURRENT_BINARY_DIR}/install"
)

set(
    BLOCK_COMPRESSOR_ZSTD_INCLUDE_DIR
    "${BLOCK_COMPRESSOR_ZSTD_PREFIX}/include"
)

set(
    BLOCK_COMPRESSOR_ZSTD_LIBRARY_DIR
    "${BLOCK_COMPRESSOR_ZSTD_PREFIX}/lib"
)

# The directories must exist at CMake configure time because they will be
# referenced by an IMPORTED target below.
file(MAKE_DIRECTORY "${BLOCK_COMPRESSOR_ZSTD_INCLUDE_DIR}")
file(MAKE_DIRECTORY "${BLOCK_COMPRESSOR_ZSTD_LIBRARY_DIR}")

set(
    BLOCK_COMPRESSOR_ZSTD_LIBRARY
    "${BLOCK_COMPRESSOR_ZSTD_LIBRARY_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}zstd${CMAKE_STATIC_LIBRARY_SUFFIX}"
)

# ---------------------------------------------------------------------------
# Build Zstd
# ---------------------------------------------------------------------------

ExternalProject_Add(
    zstd_external

    GIT_REPOSITORY
        https://github.com/facebook/zstd.git

    # Pin this in your actual project rather than following dev/master.
    GIT_TAG
        v1.5.7

    GIT_SHALLOW
        TRUE

    SOURCE_SUBDIR
        build/cmake

    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${BLOCK_COMPRESSOR_ZSTD_PREFIX}
        -DCMAKE_INSTALL_LIBDIR=lib

        # Static only.
        -DZSTD_BUILD_STATIC=ON
        -DZSTD_BUILD_SHARED=OFF

        # We only need the library.
        -DZSTD_BUILD_PROGRAMS=OFF
        -DZSTD_BUILD_TESTS=OFF
        -DZSTD_BUILD_CONTRIB=OFF

        # Propagate toolchain if the parent has one.
        -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}

        # Useful for single-configuration generators.
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}

    BUILD_BYPRODUCTS
        "${BLOCK_COMPRESSOR_ZSTD_LIBRARY}"
)

# ---------------------------------------------------------------------------
# Imported target
# ---------------------------------------------------------------------------

add_library(zstd_ext STATIC IMPORTED GLOBAL)

set_target_properties(
    zstd_ext
    PROPERTIES
        IMPORTED_LOCATION
            "${BLOCK_COMPRESSOR_ZSTD_LIBRARY}"

        INTERFACE_INCLUDE_DIRECTORIES
            "${BLOCK_COMPRESSOR_ZSTD_INCLUDE_DIR}"
)

add_dependencies(
    zstd_ext
    zstd_external
)

add_library(external::zstd ALIAS zstd_ext)