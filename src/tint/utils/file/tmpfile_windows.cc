// Copyright 2021 The Dawn & Tint Authors
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

// GEN_BUILD:CONDITION(is_win)

#include "src/tint/utils/file/tmpfile.h"

#include <stdio.h>
#include <cstdio>

namespace tint {

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

}  // namespace tint
