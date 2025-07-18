#ifndef SDB_TYPES_HPP
#define SDB_TYPES_HPP

#include <vector>
#include <array>
#include <cstddef>
#include <cstdint>

namespace sdb {
    using byte64 = std::array<std::byte, 8>;
    using byte128 = std::array<std::byte, 16>;

    // on what does a stoppoint get triggered
    enum class stoppoint_mode {
        write,
        read_write,
        execute
    };

    class virt_addr {
    public:
        virt_addr() = default;
        explicit virt_addr(std::uint64_t addr) : addr_(addr) {}

        std::uint64_t addr() const {
            return addr_;
        } 

        virt_addr operator+(std::uint64_t offset) const {
            return virt_addr(addr_ + offset);
        }

        virt_addr operator-(std::uint64_t offset) const {
            return virt_addr(addr_ - offset);
        }

        virt_addr& operator+=(std::uint64_t offset) {
            addr_ += offset;
            return *this;
        }

        virt_addr& operator-=(std::uint64_t offset) {
            addr_ -= offset;
            return *this;
        }

        bool operator==(const virt_addr& other) const {
            return addr_ == other.addr_;
        }

        bool operator!=(const virt_addr& other) const {
            return addr_ != other.addr_;
        }

        bool operator<(const virt_addr& other) const {
            return addr_ < other.addr_;
        }

        bool operator<=(const virt_addr& other) const {
            return addr_ <= other.addr_;
        }

        bool operator>(const virt_addr& other) const {
            return addr_ > other.addr_;
        }

        bool operator>=(const virt_addr& other) const {
            return addr_ >= other.addr_;
        }

    private:
        std::uint64_t addr_ = 0;
    };

    template <typename T>
    class span {
    public:
        span() = default;
        span(T* data, std::size_t size) : data_(data), size_(size) {}
        span(T* data, T* end) : data_(data), size_(end - data) {}
        template <class U>
        span(const std::vector<U>& vec) : data_(vec.data()), size_(vec.size()) {}

        T* begin() const { return data_; }
        T* end() const  { return data_ + size_; }
        std::size_t size() { return size_; }
        T& operator[](std::size_t n) { return *(data_ + n); }

    private:
        T* data_ = nullptr;
        std::size_t size_ = 0; 
    };
}

#endif