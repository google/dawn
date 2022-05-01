// Copyright 2020 The Tint Authors.
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

#ifndef SRC_TINT_WRITER_WRITER_H_
#define SRC_TINT_WRITER_WRITER_H_

#include <string>

namespace tint::writer {

/// Base class for the output writers
class Writer {
  public:
    virtual ~Writer();

    /// @returns the writer error string
    const std::string& error() const { return error_; }

    /// Converts the module into the desired format
    /// @returns true on success; false on failure
    virtual bool Generate() = 0;

  protected:
    /// Sets the error string
    /// @param msg the error message
    void set_error(const std::string& msg) { error_ = msg; }

    /// An error message, if an error was encountered
    std::string error_;
};

}  // namespace tint::writer

#endif  // SRC_TINT_WRITER_WRITER_H_
