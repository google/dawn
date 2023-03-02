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

#ifndef SRC_TINT_UTILS_IO_TMPFILE_H_
#define SRC_TINT_UTILS_IO_TMPFILE_H_

#include <string>

#include "src/tint/utils/string_stream.h"

namespace tint::utils {

/// TmpFile constructs a temporary file that can be written to, and is
/// automatically deleted on destruction.
class TmpFile {
  public:
    /// Constructor.
    /// Creates a new temporary file which can be written to.
    /// The temporary file will be automatically deleted on destruction.
    /// @param extension optional file extension to use with the file. The file
    /// have no extension by default.
    explicit TmpFile(std::string extension = "");

    /// Destructor.
    /// Deletes the temporary file.
    ~TmpFile();

    /// @return true if the temporary file was successfully created.
    operator bool() { return !path_.empty(); }

    /// @return the path to the temporary file
    std::string Path() const { return path_; }

    /// Opens the temporary file and appends |size| bytes from |data| to the end
    /// of the temporary file. The temporary file is closed again before
    /// returning, allowing other processes to open the file on operating systems
    /// that require exclusive ownership of opened files.
    /// @param data the data to write to the end of the file
    /// @param size the number of bytes to write from data
    /// @returns true on success, otherwise false
    bool Append(const void* data, size_t size) const;

    /// Appends the argument to the end of the file.
    /// @param data the data to write to the end of the file
    /// @return a reference to this TmpFile
    template <typename T>
    inline TmpFile& operator<<(T&& data) {
        utils::StringStream ss;
        ss << data;
        std::string str = ss.str();
        Append(str.data(), str.size());
        return *this;
    }

  private:
    TmpFile(const TmpFile&) = delete;
    TmpFile& operator=(const TmpFile&) = delete;

    std::string path_;
};

}  // namespace tint::utils

#endif  // SRC_TINT_UTILS_IO_TMPFILE_H_
