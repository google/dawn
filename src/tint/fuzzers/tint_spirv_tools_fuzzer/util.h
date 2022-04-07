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

#ifndef SRC_TINT_FUZZERS_TINT_SPIRV_TOOLS_FUZZER_UTIL_H_
#define SRC_TINT_FUZZERS_TINT_SPIRV_TOOLS_FUZZER_UTIL_H_

#include <sstream>
#include <string>
#include <vector>

#include "src/tint/fuzzers/tint_common_fuzzer.h"
#include "src/tint/fuzzers/tint_spirv_tools_fuzzer/mutator.h"

#include "spirv-tools/libspirv.hpp"

namespace tint::fuzzers::spvtools_fuzzer::util {

/// @param buffer will be used to output errors by the returned message
///     consumer. Must remain in scope as long as the returned consumer is in
///     scope.
/// @return the message consumer that will print errors to the `buffer`.
spvtools::MessageConsumer GetBufferMessageConsumer(std::stringstream* buffer);

/// Output errors from the SPV -> WGSL conversion.
///
/// @param message - the error message.
/// @param data - invalid SPIR-V binary.
/// @param size - the size of `data`.
/// @param error_dir - the directory, to which the binary will be printed to.
///     If it's empty, the invalid binary and supplemental files will not be
///     printed. Otherwise, it must have a `spv/` subdirectory.
void LogSpvError(const std::string& message,
                 const uint8_t* data,
                 size_t size,
                 const std::string& error_dir);

/// Output errors from the WGSL -> `output_format` conversion.
///
/// @param message - the error message.
/// @param data - the SPIR-V binary that generated the WGSL binary.
/// @param size - the size of `data`.
/// @param wgsl - the invalid WGSL binary.
/// @param output_format - the format which we attempted to convert `wgsl` to.
/// @param error_dir - the directory, to which the binary will be printed out.
///     If it's empty, the invalid binary and supplemental files will not be
///     printed. Otherwise, it must have a `wgsl/` subdirectory.
void LogWgslError(const std::string& message,
                  const uint8_t* data,
                  size_t size,
                  const std::string& wgsl,
                  OutputFormat output_format,
                  const std::string& error_dir);

/// Output errors produced by the mutator.
///
/// @param mutator - the mutator with invalid state.
/// @param error_dir - the directory, to which invalid files will be printed to.
///     If it's empty, the invalid binary and supplemental files will not be
///     printed. Otherwise, it must have a `mutator/` subdirectory.
void LogMutatorError(const Mutator& mutator, const std::string& error_dir);

/// Reads SPIR-V binary from `path` into `out`. Returns `true` if successful and
/// `false` otherwise (in this case, `out` is unchanged).
///
/// @param path - the path to the SPIR-V binary.
/// @param out - may be a `nullptr`. In this case, `false` is returned.
/// @return `true` if successful and `false` otherwise.
bool ReadBinary(const std::string& path, std::vector<uint32_t>* out);

/// Writes `binary` into `path`.
///
/// @param path - the path to write `binary` to.
/// @param binary - SPIR-V binary.
/// @return whether the operation was successful.
bool WriteBinary(const std::string& path, const std::vector<uint32_t>& binary);

}  // namespace tint::fuzzers::spvtools_fuzzer::util

#endif  // SRC_TINT_FUZZERS_TINT_SPIRV_TOOLS_FUZZER_UTIL_H_
