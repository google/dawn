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

#include "src/dawn/node/test/TestZip.h"

#include "third_party/zlib/contrib/minizip/zip.h"

namespace dawn::node::test {

bool WriteZip(const std::filesystem::path& path,
              const std::vector<ZipEntry>& entries,
              ZipMethod method) {
    zipFile zip = zipOpen64(path.string().c_str(), APPEND_STATUS_CREATE);
    if (zip == nullptr) {
        return false;
    }
    const int compression = method == ZipMethod::kDeflated ? Z_DEFLATED : 0;
    bool written = true;
    for (const ZipEntry& entry : entries) {
        zip_fileinfo info = {};
        written = written &&
                  zipOpenNewFileInZip64(zip, entry.name.c_str(), &info, nullptr, 0, nullptr, 0,
                                        nullptr, compression, Z_DEFAULT_COMPRESSION, 0) == ZIP_OK;
        written = written &&
                  (entry.contents.empty() ||
                   zipWriteInFileInZip(zip, entry.contents.data(),
                                       static_cast<unsigned>(entry.contents.size())) == ZIP_OK);
        written = written && zipCloseFileInZip(zip) == ZIP_OK;
    }
    return zipClose(zip, nullptr) == ZIP_OK && written;
}

}  // namespace dawn::node::test
