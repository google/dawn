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

#ifndef SRC_TINT_UTILS_IO_COMMAND_H_
#define SRC_TINT_UTILS_IO_COMMAND_H_

#include <string>
#include <utility>

namespace tint::utils {

/// Command is a helper used by tests for executing a process with a number of
/// arguments and an optional stdin string, and then collecting and returning
/// the process's stdout and stderr output as strings.
class Command {
  public:
    /// Output holds the output of the process
    struct Output {
        /// stdout from the process
        std::string out;
        /// stderr from the process
        std::string err;
        /// process error code
        int error_code = 0;
    };

    /// Constructor
    /// @param path path to the executable
    explicit Command(const std::string& path);

    /// Looks for an executable with the given name in the current working
    /// directory, and if not found there, in each of the directories in the
    /// `PATH` environment variable.
    /// @param executable the executable name
    /// @returns a Command which will return true for Found() if the executable
    /// was found.
    static Command LookPath(const std::string& executable);

    /// @return true if the executable exists at the path provided to the
    /// constructor
    bool Found() const;

    /// @returns the path of the command
    const std::string& Path() const { return path_; }

    /// Invokes the command with the given argument strings, blocking until the
    /// process has returned.
    /// @param args the string arguments to pass to the process
    /// @returns the process output
    template <typename... ARGS>
    Output operator()(ARGS... args) const {
        return Exec({std::forward<ARGS>(args)...});
    }

    /// Exec invokes the command with the given argument strings, blocking until
    /// the process has returned.
    /// @param args the string arguments to pass to the process
    /// @returns the process output
    Output Exec(std::initializer_list<std::string> args) const;

    /// @param input the input data to pipe to the process's stdin
    void SetInput(const std::string& input) { input_ = input; }

  private:
    std::string const path_;
    std::string input_;
};

}  // namespace tint::utils

#endif  // SRC_TINT_UTILS_IO_COMMAND_H_
