#ifndef BLOCK_COMPRESSOR_ERROR_H
#define BLOCK_COMPRESSOR_ERROR_H

#include <stdexcept>
#include <string>

namespace block_compressor
{
    inline std::string error_str(const std::string& class_name, const std::string& function_name, const std::string& msg)
    {
        return "[ERROR] " + class_name + "::" + function_name + " : " + msg;
    }

    inline std::string warning_str(const std::string& class_name, const std::string& function_name, const std::string& msg)
    {
        return "[WARNING] " + class_name + "::" + function_name + " : " + msg;
    }

    class block_compressor_error : public std::runtime_error
    {
    public:
        explicit block_compressor_error(const std::string& class_name, const std::string& function_name, const std::string& msg)
            : std::runtime_error(error_str(class_name, function_name, msg)) {}
    };
}

#endif