// Copyright 2026 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_DAWN_NODE_STANDALONE_FILEBUNDLE_H_
#define SRC_DAWN_NODE_STANDALONE_FILEBUNDLE_H_

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "src/dawn/common/Result.h"
#include "third_party/zlib/contrib/minizip/unzip.h"

namespace dawn::node::standalone {

// The files and directories of a zip archive, read on demand without extracting the archive. Paths
// are relative to the archive's root, use '/' as the separator, and the root itself is the empty
// path.
class FileBundle {
  public:
    enum class Kind {
        kNotFound,
        kFile,
        kDirectory,
    };

    // Opens the zip archive at `path`. Returns an error if it cannot be read.
    static dawn::Result<std::unique_ptr<FileBundle>, std::string> OpenZip(const std::string& path);

    ~FileBundle();
    FileBundle(const FileBundle&) = delete;
    FileBundle& operator=(const FileBundle&) = delete;

    // Returns whether `path` is a file or a directory of the bundle, or Kind::kNotFound if it is
    // neither.
    Kind Stat(std::string_view path) const;

    // Returns the contents of the file at `path`, or nullopt if `path` is not a file of the bundle
    // or its data is corrupt.
    std::optional<std::string> ReadFile(std::string_view path);

    // Returns the sorted names of the files and subdirectories directly inside the directory at
    // `path`, or nullopt if `path` is not a directory of the bundle.
    std::optional<std::vector<std::string>> ReadDir(std::string_view path) const;

  private:
    struct ZipEntry {
        unz64_file_pos position = {};
        uint64_t size = 0;
    };

    // Takes ownership of `archive`, an open unzFile.
    explicit FileBundle(unzFile archive) : archive_(archive) {}

    // Records where each file of the archive is, and what each directory holds. Returns an error if
    // the archive's central directory is corrupt.
    dawn::Result<void, std::string> ReadCentralDirectory();

    // Registers `path` and its ancestors as directories, and `path` as an entry of its parent.
    void AddDirectory(std::string_view path);

    unzFile archive_;

    // Maps the path of each file in the archive to its ZipEntry.
    absl::flat_hash_map<std::string, ZipEntry> files_;

    // Maps the path of each directory to the sorted names of the files and subdirectories directly
    // inside it.
    absl::flat_hash_map<std::string, std::vector<std::string>> directories_;
};

}  // namespace dawn::node::standalone

#endif  // SRC_DAWN_NODE_STANDALONE_FILEBUNDLE_H_
