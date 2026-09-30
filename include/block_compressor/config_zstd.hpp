#ifndef BLOCK_COMPRESSOR_CONFIG_ZSTD_H
#define BLOCK_COMPRESSOR_CONFIG_ZSTD_H

#include <block_compressor/config.hpp>

namespace block_compressor
{
    class ConfigZstd : public Config
    {
    private:
        std::int64_t preset = default_preset;
        std::uint64_t wlog = default_wlog;

    public:
        static constexpr std::int64_t default_preset = 3;
        static constexpr std::uint64_t default_wlog = 0; //use Zstd default wlog value

        ConfigZstd() = default;

        explicit ConfigZstd(const std::string& config_path) : Config(config_path) { }

        explicit ConfigZstd(const ConfigIO& config_io) : Config() { import_config(config_io); }

        virtual ~ConfigZstd() = default;

        virtual void import_config(const ConfigIO& config_io) override
        {
            set_preset(config_io.get<std::int64_t>("zstd_preset", default_preset));
            set_wlog(config_io.get<std::uint64_t>("zstd_wlog", default_wlog));

            Config::import_config(config_io); 
        }

        virtual void export_config(const std::string& config_path, ConfigIO& config_io) const override
        {
            config_io.set<std::int64_t>("zstd_preset", preset);
            config_io.set<std::uint64_t>("zstd_wlog", wlog);

            Config::export_config(config_path, config_io);
        }

        virtual std::int64_t get_preset() const { return preset; }
        virtual std::uint64_t get_wlog() const { return wlog; }

        virtual void set_preset(std::int64_t preset) { this->preset = preset; }
        virtual void set_wlog(std::uint64_t wlog) { this->wlog = wlog; }

        virtual std::string to_string() const override
        {
            std::string s = "";
            s += "preset = " + std::to_string(preset) + "\n";
            s += "wlog = " + std::to_string(wlog) + "\n";

            return Config::to_string() + s;
        }
    };
}

#endif
