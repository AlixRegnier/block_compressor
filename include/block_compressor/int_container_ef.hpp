#ifndef BLOCK_COMPRESSOR_INT_CONTAINER_EF_H
#define BLOCK_COMPRESSOR_INT_CONTAINER_EF_H

#include <vector>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>

#include <block_compressor/utils.hpp>
#include <block_compressor/int_container.hpp>

//CDS headers
#include <cds/ef.hpp>
#include <cds/io/buffer.hpp>

namespace block_compressor
{
    class custom_buffer_sink {
    public:
        [[nodiscard]] bool write(const void* data, std::size_t n) noexcept {
            std::memcpy(m_dest+offset, data, n);
            offset += n;
            return true;
        }

        [[nodiscard]] const std::byte* data() const noexcept {
            return m_dest;
        }

        [[nodiscard]] std::size_t size() const noexcept {
            return offset;
        }

        custom_buffer_sink(std::byte* dst, std::size_t offset = std::size_t{0})
            : m_dest(dst), offset(offset) {}
    private:
        char* m_dest;
        std::size_t offset;
    };

    template <typename T>
    class IntContainerEF : public IntContainer<T>
    {
    private:
        std::vector<T> integers;
        cds::ef<> ef;
        bool ef_loaded = false;
    public:
        IntContainerEF() = default;
        virtual ~IntContainerEF() = default;

        virtual void reserve(std::size_t capacity) override 
        {
            if(ef_loaded)
                throw block_compressor_error("IntContainerEF", "reserve", "Elias-Fano is loaded, you must use it");
            
            integers.reserve(capacity);
        }

        virtual void push_back(T x) override 
        {
            if(ef_loaded)
                throw block_compressor_error("IntContainerEF", "push_back", "Elias-Fano is loaded, you must use it");
                
            integers.push_back(x); 
            IntContainer<T>::increment_count(); 
        }

        virtual std::size_t upper_bound_size() const override 
        { 
            if(ef_loaded)
                return ef.memory_size();
                
            return sizeof(T) * integers.size();
        }

        virtual void deserialize_buffer(const char* data, std::size_t size) override 
        { 
            buffer_source source(data);
            ef.load(source);
        }

        virtual std::size_t serialize_buffer(char* data) const override
        {
            ef = cds::ef<>(integers);
            custom_buffer_sink sink(data);
            ef.save(sink);
            return sink.size();
        }

        virtual T get(std::size_t idx) const override 
        { 
            if(ef_loaded)
                return ef[idx]; 

            return integers[idx];
        }
    };
}

#endif