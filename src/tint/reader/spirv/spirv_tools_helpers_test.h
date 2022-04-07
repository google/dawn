// Copyright 2020 The Tint Authors
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

#ifndef SRC_TINT_READER_SPIRV_SPIRV_TOOLS_HELPERS_TEST_H_
#define SRC_TINT_READER_SPIRV_SPIRV_TOOLS_HELPERS_TEST_H_

#include <string>
#include <vector>

namespace tint::reader::spirv::test {

/// @param spirv_assembly SPIR-V assembly text
/// @returns the SPIR-V module assembled from the given text.  Numeric IDs
/// are preserved.
std::vector<uint32_t> Assemble(const std::string& spirv_assembly);

/// @param spirv_module a SPIR-V binary module
/// @returns the disassembled module
std::string Disassemble(const std::vector<uint32_t>& spirv_module);

}  // namespace tint::reader::spirv::test

#endif  // SRC_TINT_READER_SPIRV_SPIRV_TOOLS_HELPERS_TEST_H_
