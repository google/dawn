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

#ifndef SRC_WRITER_SPIRV_GENERATOR_H_
#define SRC_WRITER_SPIRV_GENERATOR_H_

#include <memory>
#include <string>
#include <vector>

#include "src/writer/writer.h"

namespace tint {

// Forward declarations
class Program;

namespace writer {
namespace spirv {

/// Forward declarations
class Builder;
class BinaryWriter;

/// Configuration options used for generating SPIR-V.
struct Options {
  /// Set to `true` to generate a PointSize builtin and have it set to 1.0
  /// from all vertex shaders in the module.
  bool emit_vertex_point_size = true;

  /// Set to `true` to disable workgroup memory zero initialization
  bool disable_workgroup_init = false;
};

/// The result produced when generating SPIR-V.
struct Result {
  /// Constructor
  Result();

  /// Destructor
  ~Result();

  /// Copy constructor
  Result(const Result&);

  /// True if generation was successful.
  bool success = false;

  /// The errors generated during code generation, if any.
  std::string error;

  /// The generated SPIR-V.
  std::vector<uint32_t> spirv;
};

/// Generate SPIR-V for a program, according to a set of configuration options.
/// The result will contain the SPIR-V, as well as success status and diagnostic
/// information.
/// @param program the program to translate to SPIR-V
/// @param options the configuration options to use when generating SPIR-V
/// @returns the resulting SPIR-V and supplementary information
Result Generate(const Program* program, const Options& options);

}  // namespace spirv
}  // namespace writer
}  // namespace tint

#endif  // SRC_WRITER_SPIRV_GENERATOR_H_
