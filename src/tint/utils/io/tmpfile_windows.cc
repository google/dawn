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

#include <stdio.h>
#include <cstdio>

namespace tint::utils {

namespace {

std::string TmpFilePath(const std::string& ext) {
    char name[L_tmpnam];
    // As we're adding an extension, to ensure the file is really unique, try
    // creating it, failing if it already exists.
    while (tmpnam_s(name, L_tmpnam - 1) == 0) {
        std::string name_with_ext = std::string(name) + ext;
        FILE* f = nullptr;
        // The "x" arg forces the function to fail if the file already exists.
        fopen_s(&f, name_with_ext.c_str(), "wbx");
        if (f) {
            fclose(f);
            return name_with_ext;
        }
    }
    return {};
}

}  // namespace

TmpFile::TmpFile(std::string ext) : path_(TmpFilePath(ext)) {}

TmpFile::~TmpFile() {
    if (!path_.empty()) {
        remove(path_.c_str());
    }
}

bool TmpFile::Append(const void* data, size_t size) const {
    FILE* file = nullptr;
    if (fopen_s(&file, path_.c_str(), "ab") != 0) {
        return false;
    }
    fwrite(data, size, 1, file);
    fclose(file);
    return true;
}

}  // namespace tint::utils
