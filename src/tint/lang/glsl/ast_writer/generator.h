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

#ifndef SRC_TINT_LANG_GLSL_AST_WRITER_GENERATOR_H_
#define SRC_TINT_LANG_GLSL_AST_WRITER_GENERATOR_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "src/tint/lang/core/builtin/access.h"
#include "src/tint/lang/glsl/ast_writer/options.h"
#include "src/tint/lang/glsl/ast_writer/version.h"
#include "src/tint/lang/wgsl/ast/pipeline_stage.h"
#include "src/tint/lang/wgsl/sem/binding_point.h"
#include "src/tint/lang/wgsl/sem/sampler_texture_pair.h"
#include "src/tint/writer/external_texture_options.h"

// Forward declarations
namespace tint {
class Program;
}  // namespace tint

namespace tint::writer::glsl {

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
/// @param entry_point the entry point to generate GLSL for
/// @returns the resulting GLSL and supplementary information
Result Generate(const Program* program, const Options& options, const std::string& entry_point);

}  // namespace tint::writer::glsl

#endif  // SRC_TINT_LANG_GLSL_AST_WRITER_GENERATOR_H_
