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

#include "src/utils/command.h"

#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>
#include <sstream>
#include <string>

namespace tint {
namespace utils {

namespace {

/// Handle is a simple wrapper around the Win32 HANDLE
class Handle {
 public:
  /// Constructor
  Handle() : handle_(nullptr) {}

  /// Constructor
  explicit Handle(HANDLE handle) : handle_(handle) {}

  /// Destructor
  ~Handle() { Close(); }

  /// Move assignment operator
  Handle& operator=(Handle&& rhs) {
    Close();
    handle_ = rhs.handle_;
    rhs.handle_ = nullptr;
    return *this;
  }

  /// Closes the handle (if it wasn't already closed)
  void Close() {
    if (handle_) {
      CloseHandle(handle_);
    }
    handle_ = nullptr;
  }

  /// @returns the handle
  operator HANDLE() { return handle_; }

  /// @returns true if the handle is not invalid
  operator bool() { return handle_ != nullptr; }

 private:
  Handle(const Handle&) = delete;
  Handle& operator=(const Handle&) = delete;

  HANDLE handle_ = nullptr;
};

/// Pipe is a simple wrapper around a Win32 CreatePipe() function
class Pipe {
 public:
  /// Constructs the pipe
  Pipe() {
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = nullptr;

    HANDLE hread;
    HANDLE hwrite;
    if (CreatePipe(&hread, &hwrite, &sa, 0)) {
      read = Handle(hread);
      write = Handle(hwrite);
      // Ensure the read handle to the pipe is not inherited
      if (!SetHandleInformation(read, HANDLE_FLAG_INHERIT, 0)) {
        read.Close();
        write.Close();
      }
    }
  }

  /// @returns true if the pipe has an open read or write file
  operator bool() { return read || write; }

  /// The reader end of the pipe
  Handle read;

  /// The writer end of the pipe
  Handle write;
};

bool ExecutableExists(const std::string& path) {
  DWORD type = 0;
  return GetBinaryTypeA(path.c_str(), &type);
}

std::string FindExecutable(const std::string& name) {
  if (ExecutableExists(name)) {
    return name;
  }
  if (ExecutableExists(name + ".exe")) {
    return name + ".exe";
  }
  if (name.find("/") == std::string::npos &&
      name.find("\\") == std::string::npos) {
    char* path_env = nullptr;
    size_t path_env_len = 0;
    if (_dupenv_s(&path_env, &path_env_len, "PATH")) {
      return "";
    }
    std::istringstream path{path_env};
    free(path_env);
    std::string dir;
    while (getline(path, dir, ';')) {
      auto test = dir + "\\" + name;
      if (ExecutableExists(test)) {
        return test;
      }
      if (ExecutableExists(test + ".exe")) {
        return test + ".exe";
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

Command::Output Command::Exec(
    std::initializer_list<std::string> arguments) const {
  Pipe stdout_pipe;
  Pipe stderr_pipe;
  Pipe stdin_pipe;
  if (!stdin_pipe || !stdout_pipe || !stderr_pipe) {
    Output output;
    output.err = "Command::Exec(): Failed to create pipes";
    return output;
  }

  if (!input_.empty()) {
    if (!WriteFile(stdin_pipe.write, input_.data(), input_.size(), nullptr,
                   nullptr)) {
      Output output;
      output.err = "Command::Exec() Failed to write stdin";
      return output;
    }
  }
  stdin_pipe.write.Close();

  STARTUPINFOA si{};
  si.cb = sizeof(si);
  si.dwFlags |= STARTF_USESTDHANDLES;
  si.hStdOutput = stdout_pipe.write;
  si.hStdError = stderr_pipe.write;
  si.hStdInput = stdin_pipe.read;

  std::stringstream args;
  args << path_;
  for (auto& arg : arguments) {
    args << " " << arg;
  }

  PROCESS_INFORMATION pi{};
  if (!CreateProcessA(nullptr,  // No module name (use command line)
                      const_cast<LPSTR>(args.str().c_str()),  // Command line
                      nullptr,  // Process handle not inheritable
                      nullptr,  // Thread handle not inheritable
                      TRUE,     // Handles are inherited
                      0,        // No creation flags
                      nullptr,  // Use parent's environment block
                      nullptr,  // Use parent's starting directory
                      &si,      // Pointer to STARTUPINFO structure
                      &pi)) {   // Pointer to PROCESS_INFORMATION structure
    Output out;
    out.err = "Command::Exec() CreateProcess() failed";
    return out;
  }

  stdout_pipe.write.Close();
  stderr_pipe.write.Close();

  Output output;

  char buf[256];
  HANDLE handles[] = {stdout_pipe.read, stderr_pipe.read};

  bool stdout_open = true;
  bool stderr_open = true;
  while (stdout_open || stderr_open) {
    auto res = WaitForMultipleObjects(2, handles, FALSE, INFINITE);
    switch (res) {
      case WAIT_FAILED:
        output.err = "Command::Exec() WaitForMultipleObjects() returned " +
                     std::to_string(res);
        return output;
      case WAIT_OBJECT_0: {  // stdout
        DWORD n = 0;
        if (ReadFile(stdout_pipe.read, buf, sizeof(buf), &n, NULL)) {
          output.out += std::string(buf, buf + n);
        } else {
          stdout_open = false;
        }
        break;
      }
      case WAIT_OBJECT_0 + 1: {  // stderr
        DWORD n = 0;
        if (ReadFile(stderr_pipe.read, buf, sizeof(buf), &n, NULL)) {
          output.err += std::string(buf, buf + n);
        } else {
          stderr_open = false;
        }
        break;
      }
    }
  }

  WaitForSingleObject(pi.hProcess, INFINITE);

  return output;
}

}  // namespace utils
}  // namespace tint
