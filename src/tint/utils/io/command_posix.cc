// Copyright 2021 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "src/tint/utils/io/command.h"

#include <sys/poll.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sstream>
#include <vector>

namespace tint::utils {

namespace {

/// File is a simple wrapper around a POSIX file descriptor
class File {
    constexpr static const int kClosed = -1;

  public:
    /// Constructor
    File() : handle_(kClosed) {}

    /// Constructor
    explicit File(int handle) : handle_(handle) {}

    /// Destructor
    ~File() { Close(); }

    /// Move assignment operator
    File& operator=(File&& rhs) {
        Close();
        handle_ = rhs.handle_;
        rhs.handle_ = kClosed;
        return *this;
    }

    /// Closes the file (if it wasn't already closed)
    void Close() {
        if (handle_ != kClosed) {
            close(handle_);
        }
        handle_ = kClosed;
    }

    /// @returns the file handle
    operator int() { return handle_; }

    /// @returns true if the file is not closed
    operator bool() { return handle_ != kClosed; }

  private:
    File(const File&) = delete;
    File& operator=(const File&) = delete;

    int handle_ = kClosed;
};

/// Pipe is a simple wrapper around a POSIX pipe() function
class Pipe {
  public:
    /// Constructs the pipe
    Pipe() {
        int pipes[2] = {};
        if (pipe(pipes) == 0) {
            read = File(pipes[0]);
            write = File(pipes[1]);
        }
    }

    /// Closes both the read and write files (if they're not already closed)
    void Close() {
        read.Close();
        write.Close();
    }

    /// @returns true if the pipe has an open read or write file
    operator bool() { return read || write; }

    /// The reader end of the pipe
    File read;

    /// The writer end of the pipe
    File write;
};

bool ExecutableExists(const std::string& path) {
    struct stat s {};
    if (stat(path.c_str(), &s) != 0) {
        return false;
    }
    return s.st_mode & S_IXUSR;
}

std::string FindExecutable(const std::string& name) {
    if (ExecutableExists(name)) {
        return name;
    }
    if (name.find("/") == std::string::npos) {
        auto* path_env = getenv("PATH");
        if (!path_env) {
            return "";
        }
        std::istringstream path{path_env};
        std::string dir;
        while (getline(path, dir, ':')) {
            auto test = dir + "/" + name;
            if (ExecutableExists(test)) {
                return test;
            }
        }
    }
    return "";
}

}  // namespace

Command::Command(const std::string& path) : path_(path) {}

Command Command::LookPath(const std::string& executable) {
    return Command(FindExecutable(executable));
}

bool Command::Found() const {
    return ExecutableExists(path_);
}

Command::Output Command::Exec(std::initializer_list<std::string> arguments) const {
    if (!Found()) {
        Output out;
        out.err = "Executable not found";
        return out;
    }

    // Pipes used for piping std[in,out,err] to / from the target process.
    Pipe stdin_pipe;
    Pipe stdout_pipe;
    Pipe stderr_pipe;

    if (!stdin_pipe || !stdout_pipe || !stderr_pipe) {
        Output output;
        output.err = "Command::Exec(): Failed to create pipes";
        return output;
    }

    // execv() and friends replace the current process image with the target
    // process image. To keep process that called this function going, we need to
    // fork() this process into a child and parent process.
    //
    // The child process is responsible for hooking up the pipes to
    // std[in,out,err]_pipes to STD[IN,OUT,ERR]_FILENO and then calling execv() to
    // run the target command.
    //
    // The parent process is responsible for feeding any input to the stdin_pipe
    // and collecting output from the std[out,err]_pipes.

    int child_id = fork();
    if (child_id < 0) {
        Output output;
        output.err = "Command::Exec(): fork() failed";
        return output;
    }

    if (child_id > 0) {
        // fork() - parent

        // Close the stdout and stderr writer pipes.
        // This is required for getting poll() POLLHUP events.
        stdout_pipe.write.Close();
        stderr_pipe.write.Close();

        // Write the input to the child process
        if (!input_.empty()) {
            ssize_t n = write(stdin_pipe.write, input_.data(), input_.size());
            if (n != static_cast<ssize_t>(input_.size())) {
                Output output;
                output.err = "Command::Exec(): write() for stdin failed";
                return output;
            }
        }
        stdin_pipe.write.Close();

        // Accumulate the stdout and stderr output from the child process
        pollfd poll_fds[2];
        poll_fds[0].fd = stdout_pipe.read;
        poll_fds[0].events = POLLIN;
        poll_fds[1].fd = stderr_pipe.read;
        poll_fds[1].events = POLLIN;

        Output output;
        bool stdout_open = true;
        bool stderr_open = true;
        while (stdout_open || stderr_open) {
            if (poll(poll_fds, 2, -1) < 0) {
                break;
            }
            char buf[256];
            if (poll_fds[0].revents & POLLIN) {
                auto n = read(stdout_pipe.read, buf, sizeof(buf));
                if (n > 0) {
                    output.out += std::string(buf, buf + n);
                }
            }
            if (poll_fds[0].revents & POLLHUP) {
                stdout_open = false;
            }
            if (poll_fds[1].revents & POLLIN) {
                auto n = read(stderr_pipe.read, buf, sizeof(buf));
                if (n > 0) {
                    output.err += std::string(buf, buf + n);
                }
            }
            if (poll_fds[1].revents & POLLHUP) {
                stderr_open = false;
            }
        }

        // Get the resulting error code
        waitpid(child_id, &output.error_code, 0);

        return output;
    } else {
        // fork() - child

        // Redirect the stdin, stdout, stderr pipes for the execv process
        if ((dup2(stdin_pipe.read, STDIN_FILENO) == -1) ||
            (dup2(stdout_pipe.write, STDOUT_FILENO) == -1) ||
            (dup2(stderr_pipe.write, STDERR_FILENO) == -1)) {
            fprintf(stderr, "Command::Exec(): Failed to redirect pipes");
            exit(errno);
        }

        // Close the pipes, once redirected above, we're now done with them.
        stdin_pipe.Close();
        stdout_pipe.Close();
        stderr_pipe.Close();

        // Run target executable
        std::vector<const char*> args;
        args.emplace_back(path_.c_str());
        for (auto& arg : arguments) {
            if (!arg.empty()) {
                args.emplace_back(arg.c_str());
            }
        }
        args.emplace_back(nullptr);
        auto res = execv(path_.c_str(), const_cast<char* const*>(args.data()));
        exit(res);
    }
}

}  // namespace tint::utils
