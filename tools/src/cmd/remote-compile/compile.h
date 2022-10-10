// Copyright 2021 The Tint Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef TOOLS_SRC_CMD_REMOTE_COMPILE_COMPILE_H_
#define TOOLS_SRC_CMD_REMOTE_COMPILE_COMPILE_H_

#include <string>

/// The return structure of a compile function
struct CompileResult {
    /// True if shader compiled
    bool success = false;
    /// Output of the compiler
    std::string output;
};

CompileResult CompileMslUsingMetalAPI(const std::string& src);

#endif  // TOOLS_SRC_CMD_REMOTE_COMPILE_COMPILE_H_
