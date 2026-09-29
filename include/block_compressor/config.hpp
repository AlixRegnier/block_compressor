#ifndef BLOCK_COMPRESSOR_CONFIG_H
#define BLOCK_COMPRESSOR_CONFIG_H

#include <block_compressor/config_io.hpp>
#include <block_compressor/utils.hpp>

namespace block_compressor
{
    class Config
    {
    private:
        std::uint64_t bits_per_element = default_bits_per_element;
        std::uint64_t elements_per_row = default_elements_per_row;
        std::uint64_t rows_per_block = default_rows_per_block;
        std::uint64_t row_size = utils::bits_to_bytes(default_bits_per_element * default_elements_per_row);
        std::uint64_t block_size = default_block_size;

        std::uint64_t targeted_rows_per_block = 0;
        std::uint64_t targeted_block_size = default_block_size;

        std::uint64_t header_size;
    public:
        static constexpr std::uint64_t default_block_size = 1 << 16; //64 kB
        static constexpr std::uint64_t default_bits_per_element = 1;
        static constexpr std::uint64_t default_elements_per_row = 8;
        static constexpr std::uint64_t default_rows_per_block = 1;
        
        Config() = default;
        explicit Config(const std::string& config_path) { import_config_file(config_path); }
        explicit Config(const ConfigIO& config_io) { import_config(config_io); }
        virtual ~Config() = default;

        void import_config_file(const std::string& config_path) { import_config(ConfigIO(config_path)); }
        virtual void import_config(const ConfigIO& config_io);

        void export_config_file(const std::string& config_path) const;
        virtual void export_config(const std::string& config_path, ConfigIO& config_io) const;

        std::size_t get_bits_per_element() const { return bits_per_element; }
        std::size_t get_elements_per_row() const { return elements_per_row; }
        std::size_t get_rows_per_block() const { return rows_per_block; }
        std::size_t get_row_size() const { return row_size; }
        std::size_t get_block_size() const { return block_size; }
        std::size_t get_targeted_block_size() const { return targeted_block_size; }
        std::size_t get_header_size() const { return header_size; }

        void set_bits_per_element(std::size_t v, bool sync = true);
        void set_elements_per_row(std::size_t v, bool sync = true);
        void target_rows_per_block(std::size_t v, bool sync = true);
        void target_block_size(std::size_t v, bool sync = true);

        void set_header_size(std::size_t v) { header_size = v; }

        virtual void sync_parameters();
        
        virtual std::string to_string() const;
    };

    inline void Config::import_config(const ConfigIO& config_io)
    {
        set_elements_per_row(config_io.get<std::uint64_t>("elements_per_row"), false);
        set_bits_per_element(config_io.get<std::uint64_t>("bits_per_element", default_bits_per_element), false);
        
        targeted_block_size = config_io.get<std::uint64_t>("target_block_size");
        targeted_rows_per_block = config_io.get<std::uint64_t>("rows_per_block");

        set_header_size(config_io.get<std::uint64_t>("header"));
        
        sync_parameters();
    }

    inline void Config::export_config_file(const std::string& config_path) const
    {
        ConfigIO c;
        export_config(config_path, c);
    }

    inline void Config::export_config(const std::string& config_path, ConfigIO& config_io) const
    {
        config_io.set<std::uint64_t>("bits_per_element", bits_per_element);
        config_io.set<std::uint64_t>("elements_per_row", elements_per_row);
        config_io.set<std::uint64_t>("block_size", block_size);
        config_io.set<std::uint64_t>("rows_per_block", rows_per_block);
        config_io.set<std::uint64_t>("header", header_size);
        config_io.write(config_path);
    }

    inline void Config::sync_parameters()
    {
        row_size = utils::bits_to_bytes(elements_per_row * bits_per_element);

        //If both targets need to be tuned
        if(!targeted_block_size == !targeted_rows_per_block)
            throw block_compressor_error("Config", "sync_parameters", "Cannot tune both the number of rows per block and the block size");

        if(targeted_rows_per_block != 0)
        {
            rows_per_block = targeted_rows_per_block;
            block_size = row_size * rows_per_block;
        }
        else
        {
            block_size = std::max(row_size, utils::nearest_multiple(targeted_block_size, row_size));
            rows_per_block = block_size / row_size;
        }
    }

    inline void Config::set_bits_per_element(std::size_t bits_per_element, bool sync)
    { 
        if(bits_per_element == 0)
            throw block_compressor_error("Config", "set_bits_per_element", "Attempted to set the number of bits per element to 0");

        this->bits_per_element = static_cast<std::uint64_t>(bits_per_element);

        if(sync)
            sync_parameters();
    }

    inline void Config::set_elements_per_row(std::size_t elements_per_row, bool sync)
    { 
        if(elements_per_row == 0)
            throw block_compressor_error("Config", "set_elements_per_row", "Attempted to set the number of elements per row to 0");

        this->elements_per_row = static_cast<std::uint64_t>(elements_per_row);

        if(sync)
            sync_parameters();
    }

    inline void Config::target_rows_per_block(std::size_t rows_per_block, bool sync)
    { 
        if(rows_per_block == 0)
            throw block_compressor_error("Config", "set_rows_per_block", "Attempted to set the number of rows per block to 0");

        targeted_rows_per_block = static_cast<std::uint64_t>(rows_per_block);
        targeted_block_size = 0;

        if(sync)
            sync_parameters();
    }

    inline void Config::target_block_size(std::size_t block_size, bool sync)
    { 
        if(targeted_block_size == 0)
            throw block_compressor_error("Config", "target_block_size", "Attempted to set the block size to 0");

        targeted_block_size = std::max(row_size, utils::nearest_multiple(block_size, row_size));
        targeted_rows_per_block = 0;
        
        if(sync)
            sync_parameters();
    }

    inline std::string Config::to_string() const
    {
        std::string s = "";
        s += "bits_per_element = " + std::to_string(bits_per_element) + "\n"; 
        s += "block_size = " + std::to_string(block_size) + "\n";
        s += "elements_per_row = " + std::to_string(elements_per_row) + "\n";
        s += "rows_per_block = " + std::to_string(rows_per_block) + "\n";
        s += "row_size = " + std::to_string(row_size) + "\n";

        return s;
    }
}

#endif