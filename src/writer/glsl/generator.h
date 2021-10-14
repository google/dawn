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

#ifndef SRC_WRITER_GLSL_GENERATOR_H_
#define SRC_WRITER_GLSL_GENERATOR_H_

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "src/ast/pipeline_stage.h"
#include "src/writer/text.h"

namespace tint {

// Forward declarations
class Program;

namespace writer {
namespace glsl {

// Forward declarations
class GeneratorImpl;

/// Configuration options used for generating GLSL.
struct Options {};

/// The result produced when generating GLSL.
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

  /// The generated GLSL.
  std::string glsl = "";

  /// The list of entry points in the generated GLSL.
  std::vector<std::pair<std::string, ast::PipelineStage>> entry_points;
};

/// Generate GLSL for a program, according to a set of configuration options.
/// The result will contain the GLSL, as well as success status and diagnostic
/// information.
/// @param program the program to translate to GLSL
/// @param options the configuration options to use when generating GLSL
/// @returns the resulting GLSL and supplementary information
Result Generate(const Program* program,
                const Options& options,
                const std::string& entry_point);

}  // namespace glsl
}  // namespace writer
}  // namespace tint

#endif  // SRC_WRITER_GLSL_GENERATOR_H_
