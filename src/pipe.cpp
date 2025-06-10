#include <unistd.h>
#include <fcntl.h>
#include <utility>
#include <libsdb/error.hpp>
#include <libsdb/pipe.hpp>

sdb::pipe::pipe(bool close_on_exec) {
    if (pipe2(fds_, close_on_exec? O_CLOEXEC : 0) < 0) {
        error::send_error("pipe creation failed");
    }
}

sdb::pipe::~pipe() {
    close_read();
    close_write();
}

int sdb::pipe::release_read() {
    return std::exchange(fds_[read_fd_], -1);
}

int sdb::pipe::release_write() {
    return std::exchange(fds_[write_fd_], -1);
}

void sdb::pipe::close_read() {
    if (fds_[read_fd_] != -1) {
        close(fds_[read_fd_]);
        fds_[read_fd_] = -1;
    }
}

void sdb::pipe::close_write() {
    if (fds_[write_fd_] != -1) {
        close(fds_[write_fd_]);
        fds_[write_fd_] = -1;
    }
}

std::vector<std::byte> sdb::pipe::read() {
    char buff[read_size_];
    int chars_read;

    if ((chars_read = ::read(fds_[read_fd_], buff, sizeof(buff))) < 0) {
        error::send_error("Could not read from pipe");
    }

    auto bytes = reinterpret_cast<std::byte*>(buff);
    return std::vector<std::byte>(bytes, bytes + chars_read);
}

void sdb::pipe::write(std::byte* from, std::size_t bytes) {
    if (::write(fds_[write_fd_], from, bytes) < 0) {
        error::send_error("could not write to pipe");
    }
}