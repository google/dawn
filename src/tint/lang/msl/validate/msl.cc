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

#include "src/tint/lang/msl/validate/val.h"

#include "src/tint/lang/wgsl/ast/module.h"
#include "src/tint/lang/wgsl/program/program.h"
#include "src/tint/utils/command/command.h"
#include "src/tint/utils/file/tmpfile.h"

namespace tint::msl::validate {

Result Msl(const std::string& xcrun_path, const std::string& source, MslVersion version) {
    Result result;

    auto xcrun = tint::Command(xcrun_path);
    if (!xcrun.Found()) {
        result.output = "xcrun not found at '" + std::string(xcrun_path) + "'";
        result.failed = true;
        return result;
    }

    tint::TmpFile file(".metal");
    file << source;

    const char* version_str = nullptr;
    switch (version) {
        case MslVersion::kMsl_1_2:
            version_str = "-std=macos-metal1.2";
            break;
        case MslVersion::kMsl_2_1:
            version_str = "-std=macos-metal2.1";
            break;
    }

#ifdef _WIN32
    // On Windows, we should actually be running metal.exe from the Metal
    // Developer Tools for Windows
    auto res = xcrun("-x", "metal",  //
                     "-o", "NUL",    //
                     version_str,    //
                     "-c", file.Path());
#else
    auto res = xcrun("-sdk", "macosx", "metal",  //
                     "-o", "/dev/null",          //
                     version_str,                //
                     "-c", file.Path());
#endif
    if (!res.out.empty()) {
        if (!result.output.empty()) {
            result.output += "\n";
        }
        result.output += res.out;
    }
    if (!res.err.empty()) {
        if (!result.output.empty()) {
            result.output += "\n";
        }
        result.output += res.err;
    }
    result.failed = (res.error_code != 0);

    return result;
}

}  // namespace tint::msl::validate
