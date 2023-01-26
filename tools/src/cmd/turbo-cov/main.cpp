// Copyright 2022 The Tint and Dawn Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// turbo-cov is a minimal re-implementation of LLVM's llvm-cov, that emits just
// the per segment coverage in a binary stream. This avoids the overhead of
// encoding to JSON.

#include <cstdio>
#include <utility>

#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ProfileData/Coverage/CoverageMapping.h"
#include "llvm/ProfileData/InstrProfReader.h"

#if defined(_MSC_VER)
#include <fcntl.h>  // _O_BINARY
#include <io.h>     // _setmode
#endif

namespace {

template <typename T>
void emit(T v) {
    fwrite(&v, sizeof(v), 1, stdout);
}

void emit(const llvm::StringRef& str) {
    uint64_t len = str.size();
    emit<uint32_t>(len);
    fwrite(str.data(), len, 1, stdout);
}

}  // namespace

int main(int argc, const char** argv) {
#if defined(_MSC_VER)
    // Change stdin & stdout from text mode to binary mode.
    // This ensures sequences of \r\n are not changed to \n.
    _setmode(_fileno(stdin), _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);
#endif

    if (argc < 3) {
        fprintf(stderr, "turbo-cov <exe> <profdata>\n");
        return 1;
    }

    auto exe = argv[1];
    auto profdata = argv[2];

    auto res = llvm::coverage::CoverageMapping::load({exe}, profdata);
    if (auto E = res.takeError()) {
        fprintf(stderr, "failed to load executable '%s': %s\n", exe,
                llvm::toString(std::move(E)).c_str());
        return 1;
    }

    auto coverage = std::move(res.get());
    if (!coverage) {
        fprintf(stderr, "failed to load coverage information\n");
        return 1;
    }

    if (auto mismatched = coverage->getMismatchedCount()) {
        fprintf(stderr, "%d functions have mismatched data\n", static_cast<int>(mismatched));
        return 1;
    }

    // uint32 num_files
    //   file[0]
    //     uint32 filename.length
    //     <data> filename.data
    //     uint32 num_segments
    //       file[0].segment[0]
    //         uint32 line
    //         uint32 col
    //         uint32 count
    //         uint8  hasCount
    //       file[0].segment[1]
    //         ...
    //   file[1]
    //     ...

    auto files = coverage->getUniqueSourceFiles();
    emit<uint32_t>(files.size());
    for (auto& file : files) {
        emit(file);
        auto fileCoverage = coverage->getCoverageForFile(file);
        emit<uint32_t>(fileCoverage.end() - fileCoverage.begin());
        for (auto& segment : fileCoverage) {
            emit<uint32_t>(segment.Line);
            emit<uint32_t>(segment.Col);
            emit<uint32_t>(segment.Count);
            emit<uint8_t>(segment.HasCount ? 1 : 0);
        }
    }

    return 0;
}
