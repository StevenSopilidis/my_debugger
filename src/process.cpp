#include <libsdb/process.hpp>
#include <libsdb/error.hpp>
#include <libsdb/pipe.hpp>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace {
    void exit_with_error(sdb::pipe& channel, const std::string& prefix) {
        auto msg = prefix + ": " + std::strerror(errno);
        channel.write(reinterpret_cast<std::byte*>(msg.data()), msg.size());
        exit(-1);
    }
}

std::unique_ptr<sdb::process>
sdb::process::launch(std::filesystem::path path, bool debug)
{
    pipe channel(true);

    pid_t pid;
    if ((pid = fork()) < 0) {
        error::send_error("fork() failed");
    }

    if (pid == 0)
    {
        // child process
        channel.close_read();
        if (debug && ptrace(PTRACE_TRACEME, 0, nullptr, nullptr) < 0) {
            exit_with_error(channel, "tracing failed");
        }

        if (execlp(path.c_str(), path.c_str(), nullptr) < 0) {
            exit_with_error(channel, "exec() failed");
        }
    }

    // parent process
    channel.close_write();
    auto data = channel.read();

    if (data.size() > 0) {
        // process sent data, something went wrong
        waitpid(pid, nullptr, 0);
        auto chars = reinterpret_cast<char*>(data.data());
        error::send_error(std::string(chars, chars + data.size()));
    }

    std::unique_ptr<process> proc(new process(pid, true, debug));

    if (debug) {
        proc->wait_on_signal();
    }
    
    return proc;
}

std::unique_ptr<sdb::process>
sdb::process::attach(pid_t pid) {
    if (pid == 0) {
        error::send("invalid pid");
    }

    if (ptrace(PTRACE_ATTACH, pid, nullptr, nullptr) < 0) {
        error::send_error("could not attach");
    }

    std::unique_ptr<sdb::process> proc(new process(pid, false, true));
    proc->wait_on_signal();
    return proc;
}

sdb::process::~process() {
    if (pid_ != 0) {
        int status;

        if (is_attached_) {
            if (state_ == process_state::running) {
                kill(pid_, SIGSTOP); // send stop signal
                waitpid(pid_, &status, 0); // wait for process to receive it
            }
    
            ptrace(PTRACE_DETACH, pid_, nullptr, nullptr); // detach process
            kill(pid_, SIGCONT); // let it continue
        }

        if (terminate_on_end_) { // if we created the process, kill it
            kill(pid_, SIGKILL);
            waitpid(pid_, &status, 0);
        }
    }
}

void sdb::process::resume() {
    if (ptrace(PTRACE_CONT, pid_, nullptr, nullptr) < 0) {
        error::send_error("could not resume");
    }

    state_ = process_state::running;
}

sdb::stop_reason::stop_reason(int wait_status) {
    if (WIFEXITED(wait_status)) {
        reason = process_state::exited;
        info = WIFEXITED(wait_status);
    }
    else if (WIFSIGNALED(wait_status)) {
        reason = process_state::terminated;
        info = WTERMSIG(wait_status);
    }
    else if (WIFSTOPPED(wait_status)) {
        reason = process_state::stopped;
        info = WSTOPSIG(wait_status);
    }
}

sdb::stop_reason sdb::process::wait_on_signal() {
    int wait_status;
    int options = 0;

    if (waitpid(pid_, &wait_status, options) < 0) {
        error::send_error("waitpid() failed");
    }

    stop_reason reason(wait_status);
    state_ = reason.reason;

    // if processes haulted read register state
    if (is_attached_ and state_ == process_state::stopped)
        read_all_registers();

    return reason;
}

void sdb::process::read_all_registers() {
    if (ptrace(PTRACE_GETREGS, pid_, &get_registers().data_.regs) < 0) {
        error::send_error("could not read GPR registers");
    }

    if (ptrace(PTRACE_GETFPREGS, pid_, nullptr, &get_registers().data_.regs) < 0) {
        error::send_error("could not read FPR registers");
    }

    // read 8 debug registers
    for (int i = 0; i < 8; i++) {
        auto id = static_cast<int>(register_id::dr0) + i;
        auto info = register_info_by_id(static_cast<register_id>(id));

        errno = 0;
        std::int64_t data = ptrace(PTRACE_PEEKUSER, pid_, info.offset, nullptr);
        if (errno != 0)
            error::send_error("could not read debug register");

        get_registers().data_.u_debugreg[i] = data;
    }
}

void sdb::process::write_user_area(std::size_t offset, std::uint64_t data) {
    if (ptrace(PTRACE_POKEUSER, pid_, offset, data) < 0)
        error::send_error("could not write to user area");
}

void sdb::process::write_fprs(const user_fpregs_struct* fprs) {
    if (ptrace(PTRACE_SETFPREGS, pid_, nullptr, &fprs) < 0) {
        error::send_error("could not write floating point registers");
    }
}

void sdb::process::write_gprs(const user_regs_struct* gprs) {
    if (ptrace(PTRACE_SETREGS, pid_, nullptr, &gprs) < 0) {
        error::send_error("could not write general purpose registers");
    }
}