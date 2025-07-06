#ifndef SDB_BREAKPOINT_SITE_HPP
#define SDB_BREAKPOINT_SITE_HPP

#include <cstddef>
#include <cstdint>
#include <libsdb/types.hpp>

namespace sdb {
    class process;

    // represents software break point at physical address
    class breakpoint_site {
    public:
        breakpoint_site() = delete;
        breakpoint_site(const breakpoint_site&) = delete;
        breakpoint_site& operator=(const breakpoint_site&) = delete;

        using id_type = std::uint32_t;
        id_type id() const { return id_; }

        void enable();
        void disable();

        bool is_enabled() const { return is_enabled_; }
        virt_addr address() const { return address_; }
        
        bool at_address(virt_addr addr) const {
            return address_ == addr;
        }

        bool in_range(virt_addr low, virt_addr high) const {
            return low <= address_ and address_ <= high;
        }

    private:
        breakpoint_site(process& proc, virt_addr addr);
        friend process;
    
        id_type id_; // unique id for break point
        bool is_enabled_; 
        virt_addr address_;
        std::byte saved_data_; // saved instructions that will be replaced by int3 for the breaking
        process* process_;
    };
}


#endif