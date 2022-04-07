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

#ifndef SRC_TINT_READER_WGSL_PARSER_H_
#define SRC_TINT_READER_WGSL_PARSER_H_

#include "src/tint/program.h"

namespace tint::reader::wgsl {

/// Parses the WGSL source, returning the parsed program.
/// If the source fails to parse then the returned
/// `program.Diagnostics.contains_errors()` will be true, and the
/// `program.Diagnostics()` will describe the error.
/// @param file the source file
/// @returns the parsed program
Program Parse(Source::File const* file);

}  // namespace tint::reader::wgsl

#endif  // SRC_TINT_READER_WGSL_PARSER_H_
