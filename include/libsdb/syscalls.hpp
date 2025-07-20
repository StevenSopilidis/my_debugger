#ifndef SDB_SYSCALL_HPP
#define SDB_SYSCALL_HPP

#include <string_view>

namespace sdb {
    std::string_view syscall_id_to_name(int id);
    int syscall_name_to_id(std::string_view id);
}

#endif