// Copyright 2020 The Tint Authors.
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

#ifndef SRC_TINT_READER_SPIRV_PARSER_H_
#define SRC_TINT_READER_SPIRV_PARSER_H_

#include <vector>

#include "src/tint/program.h"

namespace tint::reader::spirv {

/// Options that control how the SPIR-V parser should behave.
struct Options {
    /// Set to `true` to allow calls to derivative builtins in non-uniform control flow.
    bool allow_non_uniform_derivatives = false;
};

/// Parses the SPIR-V source data, returning the parsed program.
/// If the source data fails to parse then the returned
/// `program.Diagnostics.contains_errors()` will be true, and the
/// `program.Diagnostics()` will describe the error.
/// @param input the source data
/// @param options the parser options
/// @returns the parsed program
Program Parse(const std::vector<uint32_t>& input, const Options& options = {});

}  // namespace tint::reader::spirv

#endif  // SRC_TINT_READER_SPIRV_PARSER_H_
