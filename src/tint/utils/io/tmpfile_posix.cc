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

#include "src/tint/utils/io/tmpfile.h"

#include <unistd.h>
#include <limits>

#include "src/tint/debug.h"

namespace tint::utils {

namespace {

std::string TmpFilePath(std::string ext) {
    char const* dir = getenv("TMPDIR");
    if (dir == nullptr) {
        dir = "/tmp";
    }

    // mkstemps requires an `int` for the file extension name but STL represents
    // size_t. Pre-C++20 there the behavior for unsigned-to-signed conversion
    // (when the source value exceeds the representable range) is implementation
    // defined. While such a large file extension is unlikely in practice, we
    // enforce this here at runtime.
    TINT_ASSERT(Utils, ext.length() <= static_cast<size_t>(std::numeric_limits<int>::max()));
    std::string name = std::string(dir) + "/tint_XXXXXX" + ext;
    int file = mkstemps(&name[0], static_cast<int>(ext.length()));
    if (file != -1) {
        close(file);
        return name;
    }
    return "";
}

}  // namespace

TmpFile::TmpFile(std::string extension) : path_(TmpFilePath(std::move(extension))) {}

TmpFile::~TmpFile() {
    if (!path_.empty()) {
        remove(path_.c_str());
    }
}

bool TmpFile::Append(const void* data, size_t size) const {
    if (auto* file = fopen(path_.c_str(), "ab")) {
        fwrite(data, size, 1, file);
        fclose(file);
        return true;
    }
    return false;
}

}  // namespace tint::utils
