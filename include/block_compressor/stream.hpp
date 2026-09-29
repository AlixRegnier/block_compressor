#ifndef BLOCK_COMPRESSOR_STREAM_H
#define BLOCK_COMPRESSOR_STREAM_H

#include <iostream>
#include <memory>
#include <string>

#include <block_compressor/utils.hpp>

namespace block_compressor
{
    class InputStream
    {
    public:
        // Borrow an existing stream.
        explicit InputStream(std::istream& stream) { }

        // Own a file stream created from a path.
        explicit InputStream(const std::string& filename)
            : _owned_stream(std::make_unique<std::ifstream>(filename)), _stream(_owned_stream.get()) { }

        std::istream& stream()
        {
            return *_stream;
        }

        bool valid() const
        {
            return (bool)_stream;
        }

    private:
        std::unique_ptr<std::istream> _owned_stream;
        std::istream* _stream;
    };

    class OutputStream
    {
    public:
        // Borrow an existing stream.
        explicit OutputStream(std::ostream& stream)
            : _stream(&stream) { }

        // Own a file stream created from a path.
        explicit OutputStream(const std::string& filename)
            : _owned_stream(std::make_unique<std::ofstream>(filename)), _stream(_owned_stream.get()) { }

        std::ostream& stream()
        {
            return *_stream;
        }
        
        bool valid() const
        {
            return (bool)_stream;
        }

    private:
        std::unique_ptr<std::ostream> _owned_stream;
        std::ostream* _stream;
    };
}

#endif