include(ExternalProject)

set(
    BLOCK_COMPRESSOR_CDS_PREFIX
    "${CMAKE_CURRENT_BINARY_DIR}/install"
)

set(
    BLOCK_COMPRESSOR_CDS_INCLUDE_DIR
    "${BLOCK_COMPRESSOR_CDS_PREFIX}/include"
)

file(MAKE_DIRECTORY "${BLOCK_COMPRESSOR_CDS_INCLUDE_DIR}")

ExternalProject_Add(
    cds_ext

    GIT_REPOSITORY
        https://github.com/tlemane/cds.git

    GIT_TAG main

    GIT_SHALLOW
        TRUE

    CONFIGURE_COMMAND
        ""

    BUILD_COMMAND
        ""

    INSTALL_COMMAND
        ${CMAKE_COMMAND} -E copy_directory
        <SOURCE_DIR>/include
        ${BLOCK_COMPRESSOR_CDS_INCLUDE_DIR}
)

add_library(cds_ext INTERFACE)

target_include_directories(
    cds_ext
    INTERFACE
        "${BLOCK_COMPRESSOR_CDS_INCLUDE_DIR}"
)

target_compile_features(
    cds_ext
    INTERFACE
        cxx_std_23
)

add_dependencies(
    cds_ext
    cds_external
)

add_library(external::cds ALIAS cds_ext)