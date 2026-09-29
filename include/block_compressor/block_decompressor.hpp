#ifndef BLOCK_COMPRESSOR_BLOCK_DECOMPRESSOR_H
#define BLOCK_COMPRESSOR_BLOCK_DECOMPRESSOR_H

#include <fcntl.h>
#include <unistd.h>
#include <vector>
#include <algorithm>
#include <sys/mman.h>

#include <block_compressor/decompressor.hpp>
#include <block_compressor/int_container.hpp>

namespace block_compressor
{
    class BlockDecompressor
    {
    private:
        char* block;

        Decompressor* decompressor;

        std::size_t current_block_index = ~std::size_t{0};
        std::size_t current_block_size = ~std::size_t{0};
        std::size_t block_size;
        std::size_t nb_blocks;

        //IO variables
        bool owned = false;
        int file_descriptor = -1;
        const char* map = nullptr; //mmapped file
        void* __map = nullptr;
        std::size_t map_size = 0;

        IntContainer<std::uint64_t>* int_container;

        
    public:
        //Copy is deleted
        BlockDecompressor(const BlockDecompressor&) = delete;
        BlockDecompressor& operator=(const BlockDecompressor&) = delete;

        //Move is allowed
        BlockDecompressor(BlockDecompressor&&) = default;
        BlockDecompressor& operator=(BlockDecompressor&&) = default;

        explicit BlockDecompressor(const char* input, std::size_t input_size, std::size_t block_size, Decompressor& decompressor, IntContainer<std::uint64_t>& int_container, std::size_t offset = 0);
        explicit BlockDecompressor(const std::string& input_path, std::size_t block_size, Decompressor& decompressor, IntContainer<std::uint64_t>& int_container, std::size_t offset = 0);
        virtual ~BlockDecompressor();

        std::size_t decompress_block(std::size_t idx);
        std::size_t decompress_block(std::size_t idx, char* output);

        //Return current decompressed block
        const char* get_block() { return block; }

        std::size_t get_current_block_index() const { return current_block_index; }
        std::size_t get_current_block_size() const { return current_block_size; }
        std::size_t get_block_size() const { return block_size; }
        std::size_t get_nb_blocks() const { return nb_blocks; }

        //Return a pointer 
        const char* get_row(std::uint64_t hash, std::size_t row_size);

        //Decompress all blocks
        std::size_t decompress_all(const std::string& output_path, int mode = 0644);
        std::size_t decompress_all(std::ostream& output_stream);
        std::size_t decompress_all(char* output);
    };

    inline BlockDecompressor::BlockDecompressor(const std::string& input_path, std::size_t block_size, Decompressor& decompressor, IntContainer<std::uint64_t>& int_container, std::size_t offset)
        : block_size(block_size), owned(true), decompressor(&decompressor), int_container(&int_container)
    {
        file_descriptor = open(input_path.c_str(), O_RDONLY);

        if(file_descriptor < 0)
            throw block_compressor_error("BlockDecompressor", "()", "Open syscall failed on: '" + input_path + "'");

        map_size = lseek(file_descriptor, 0, SEEK_END);

        if(map_size == 0)
        {
            close(file_descriptor);
            throw block_compressor_error("BlockDecompressor", "()", "File is empty: '" + input_path + "'");
        }

        __map = mmap(nullptr, map_size, PROT_READ, MAP_PRIVATE, file_descriptor, 0);

        if(__map == MAP_FAILED)
        {
            close(file_descriptor);
            throw block_compressor_error("BlockDecompressor", "()", "mmap failed, OOM ?");
        }

        map = static_cast<const char*>(__map) + offset;

        nb_blocks = int_container.size() - 1;
        block = new char[block_size];
    }

    inline BlockDecompressor::BlockDecompressor(const char* input, std::size_t input_size, std::size_t block_size, Decompressor& decompressor, IntContainer<std::uint64_t>& int_container, std::size_t offset)
        : block_size(block_size), owned(false), decompressor(&decompressor), int_container(&int_container), map(input+offset), map_size(input_size-offset) 
    { 
        nb_blocks = int_container.size() - 1;
        block = new char[block_size];
    }

    inline BlockDecompressor::~BlockDecompressor()
    {
        if(owned && __map != nullptr)
        {
            munmap(__map, map_size);
            close(file_descriptor);
        }

        if(block != nullptr)
            delete[] block;

        owned = false;
        __map = nullptr;
        map = nullptr;
        block = nullptr;
    }

    inline std::size_t BlockDecompressor::decompress_block(std::size_t idx)
    {
        if(idx >= nb_blocks)
            throw block_compressor_error("BlockDecompressor", "decompress_block", "Required block is out of range");

        if(current_block_index == idx)
            return current_block_size;

        std::uint64_t a = int_container->get(idx);
        std::uint64_t b = int_container->get(idx+1);

        std::size_t written_bytes = decompressor->decompress(map+a, block, b-a, block_size);

        //Throw exception if last block is smaller than block_size OR a given block is greater than
        if(idx+1 != nb_blocks && written_bytes < block_size || written_bytes > block_size)
            throw block_compressor_error("BlockDecompressor", "decompress_block", "Decompressed block expected size is '" + std::to_string(block_size) + "', got: '" + std::to_string(written_bytes) + "'");

        current_block_index = idx;
        current_block_size = written_bytes;

        return written_bytes;
    }

    inline std::size_t BlockDecompressor::decompress_block(std::size_t idx, char* output)
    {
        if(idx >= nb_blocks)
            throw block_compressor_error("BlockDecompressor", "decompress_block", "Required block is out of range");

        if(current_block_index == idx)
        {
            std::memcpy(output, block, current_block_size);
            return current_block_size;
        }

        std::uint64_t a = int_container->get(idx);
        std::uint64_t b = int_container->get(idx+1);

        std::size_t written_bytes = decompressor->decompress(map+a, output, b-a, block_size);

        //Throw exception if block is smaller than (without being the last block) OR a the block is greater than expected
        if(idx+1 != nb_blocks && written_bytes < block_size || written_bytes > block_size)
            throw block_compressor_error("BlockDecompressor", "decompress_block", "Decompressed block expected size is '" + std::to_string(block_size) + "', got: '" + std::to_string(written_bytes) + "'");

        return written_bytes;
    }

    inline const char* BlockDecompressor::get_row(std::uint64_t row_idx, std::size_t row_size)
    {
        std::size_t rows_per_block = block_size / row_size;
        std::size_t block_idx = row_idx / rows_per_block;
        std::size_t row = row_idx % rows_per_block;

        decompress_block(block_idx);
        return block + row;
    }

    inline std::size_t BlockDecompressor::decompress_all(const std::string& output_path, int mode)
    {
        int fd = open(output_path.c_str(), O_CREAT | O_TRUNC | O_RDWR, mode);

        if(fd < 0)
            throw block_compressor_error("BlockDecompressor", "decompress_all", "Open syscall failed on: '" + output_path + "'");

        std::size_t max_size = nb_blocks * block_size;
        char* file_map = (char*)mmap(nullptr, max_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

        if(file_map == MAP_FAILED)
        {
            close(fd);
            throw block_compressor_error("BlockDecompressor", "decompress_all", "mmap failed, OOM ?");
        }

        std::size_t written_bytes = decompress_all(file_map);

        munmap(file_map, max_size);

        if(written_bytes < max_size && ftruncate(fd, written_bytes) < 0)
        {
            close(fd);
            throw block_compressor_error("BlockDecompressor", "decompress_all", "Could not truncate output file: '" + output_path + "'");
        }

        close(fd);

        return written_bytes;
    }

    inline std::size_t BlockDecompressor::decompress_all(char* output)
    {
        std::size_t offset = 0;
        for(std::size_t i = 0; i < nb_blocks; ++i)
            offset += decompress_block(i, output+offset);

        return offset;
    }

    inline std::size_t BlockDecompressor::decompress_all(std::ostream& output_stream)
    {
        std::size_t offset = 0;
        for(std::size_t i = 0; i < nb_blocks; ++i)
        {
            offset += decompress_block(i);
            output_stream.write(block, static_cast<std::streamsize>(current_block_size));
        }

        return offset;
    }
}
#endif
