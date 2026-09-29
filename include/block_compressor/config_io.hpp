#ifndef BLOCK_COMPRESSOR_CONFIG_IO_H
#define BLOCK_COMPRESSOR_CONFIG_IO_H

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits>
#include <regex>
#include <type_traits>
#include <string>
#include <unordered_map>
#include <variant>

#include <block_compressor/utils.hpp>

namespace block_compressor
{
    class ConfigIO
    {
    private:
        using Value = std::variant<double, std::int64_t, std::uint64_t, std::string>;
        std::unordered_map<std::string, Value> properties;

    public:
        ConfigIO() = default;
        explicit ConfigIO(const std::string& path);
        
        virtual void read(const std::string& path);
        virtual void write(const std::string& path) const;

        template <class T>
        const T& get(const std::string& property, const T& default_value = T{0}) const
        {
            auto it = properties.find(property);

            if(it == properties.end())
                return default_value;

            return std::get<T>(it->second);
        }

        template <class T>
        void set(const std::string& property, const T& value)
        {
            properties[property] = value;
        }
    };

    inline ConfigIO::ConfigIO(const std::string& path) 
    { 
        read(path);
    }

    inline void ConfigIO::read(const std::string& path)
    {
        std::ifstream config_file(path);

        if(!config_file)
            throw block_compressor_error("ConfigIO", "read", "Given file does not exist: '" + path + "'");

        const std::regex propertyRegex(
            R"(^\s*([A-Za-z_-]+)\s*[=:]\s*(.*?)\s*$)"
        );

        const std::regex integerRegex(
            R"(^[+-]?[0-9]+$)"
        );

        const std::regex floatingRegex(
            R"(^[+-]?(?:[0-9]+\.[0-9]*|[0-9]*\.[0-9]+)(?:[eE][+-]?[0-9]+)?$)"
        );

        std::size_t line_no = 0;
        std::string line;

        while(std::getline(config_file, line))
        {
            ++line_no;

            // Skip empty / whitespace-only lines.
            if(line.find_first_not_of(" \t\r\n") == std::string::npos)
                continue;

            std::smatch match;

            if(!std::regex_match(line, match, propertyRegex))
                throw block_compressor_error("ConfigIO", "read", "Invalid property at line " + std::to_string(line_no));

            std::string key   = match[1].str();
            std::string value = match[2].str();

            // String
            if(value.size() >= 2 &&
                ((value.front() == '"'  && value.back() == '"') ||
                (value.front() == '\'' && value.back() == '\'')))
            {
                properties[key] = value.substr(1, value.size() - 2);
            }
            // Integer
            else if(std::regex_match(value, integerRegex))
            {
                try
                {
                    if (!value.empty() && value.front() == '-')
                        properties[key] = static_cast<std::int64_t>(std::stoll(value));
                    else
                        properties[key] = static_cast<std::uint64_t>(std::stoull(value));
                }
                catch (const std::exception&)
                {
                    throw block_compressor_error("ConfigIO", "read", "Integer out of range at line " + std::to_string(line_no));
                }
            }
            // Floating point
            else if(std::regex_match(value, floatingRegex))
            {
                properties[key] = std::stod(value);
            }
            else
            {
                throw block_compressor_error("ConfigIO", "read", "Invalid value at line " + std::to_string(line_no));
            }
        }
    }

    inline void ConfigIO::write(const std::string& path) const
    {
        std::ofstream file(path);

        if (!file)
            throw block_compressor_error("ConfigIO", "write", "Could not open configuration file: '" + path + "'");

        std::vector<std::string> sorted_keys;
        sorted_keys.reserve(properties.size());

        for (const auto& [key, _] : properties)
            sorted_keys.push_back(key);

        std::sort(sorted_keys.begin(), sorted_keys.end());

        for (const auto& key : sorted_keys)
        {
            const auto& value = properties.at(key);

            file << key << " = ";

            std::visit(
                [&file](const auto& v)
                {
                    using T = std::decay_t<decltype(v)>;

                    if constexpr (std::is_same_v<T, std::string>)
                        file << std::quoted(v);
                    else if constexpr (std::is_same_v<T, double>)
                        file << std::setprecision(17) << v;
                    else
                        file << v;
                },
                value
            );

            file << '\n';
        }

        if (!file)
            throw block_compressor_error("ConfigIO", "write", "Could not write configuration file: '" + path + "'");
    }
}

#endif