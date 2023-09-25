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

#ifndef SRC_TINT_LANG_WGSL_READER_READER_H_
#define SRC_TINT_LANG_WGSL_READER_READER_H_

#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/wgsl/program/program.h"

namespace tint::wgsl::reader {

/// Parses the WGSL source, returning the parsed program.
/// If the source fails to parse then the returned
/// `program.Diagnostics.contains_errors()` will be true, and the
/// `program.Diagnostics()` will describe the error.
/// @param file the source file
/// @returns the parsed program
Program Parse(const Source::File* file);

/// Parse a WGSL program from source, and return an IR module.
/// @param file the input WGSL file
/// @returns the resulting IR module, or failure
Result<core::ir::Module> WgslToIR(const Source::File* file);

}  // namespace tint::wgsl::reader

#endif  // SRC_TINT_LANG_WGSL_READER_READER_H_
