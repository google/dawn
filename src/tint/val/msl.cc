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

#include "src/tint/val/val.h"

#include "src/tint/ast/module.h"
#include "src/tint/program.h"
#include "src/tint/utils/io/command.h"
#include "src/tint/utils/io/tmpfile.h"

namespace tint::val {

Result Msl(const std::string& xcrun_path, const std::string& source) {
    Result result;

    auto xcrun = utils::Command(xcrun_path);
    if (!xcrun.Found()) {
        result.output = "xcrun not found at '" + std::string(xcrun_path) + "'";
        result.failed = true;
        return result;
    }

    utils::TmpFile file(".metal");
    file << source;

#ifdef _WIN32
    // On Windows, we should actually be running metal.exe from the Metal
    // Developer Tools for Windows
    auto res = xcrun("-x", "metal",        //
                     "-o", "NUL",          //
                     "-std=osx-metal1.2",  //
                     "-c", file.Path());
#else
    auto res = xcrun("-sdk", "macosx", "metal",  //
                     "-o", "/dev/null",          //
                     "-std=osx-metal1.2",        //
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

}  // namespace tint::val
