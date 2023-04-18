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

#ifndef SRC_TINT_WRITER_HLSL_GENERATOR_H_
#define SRC_TINT_WRITER_HLSL_GENERATOR_H_

#include <bitset>
#include <memory>
#include <optional>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "src/tint/ast/pipeline_stage.h"
#include "src/tint/reflection.h"
#include "src/tint/sem/binding_point.h"
#include "src/tint/utils/bitset.h"
#include "src/tint/writer/array_length_from_uniform_options.h"
#include "src/tint/writer/binding_remapper_options.h"
#include "src/tint/writer/external_texture_options.h"
#include "src/tint/writer/text.h"

// Forward declarations
namespace tint {
class Program;
}  // namespace tint

namespace tint::writer::hlsl {

/// Configuration options used for generating HLSL.
struct Options {
    /// Constructor
    Options();
    /// Destructor
    ~Options();
    /// Copy constructor
    Options(const Options&);
    /// Copy assignment
    /// @returns this Options
    Options& operator=(const Options&);

    /// Set to `true` to disable software robustness that prevents out-of-bounds accesses.
    bool disable_robustness = false;

    /// The binding point to use for information passed via root constants.
    std::optional<sem::BindingPoint> root_constant_binding_point;

    /// Set to `true` to disable workgroup memory zero initialization
    bool disable_workgroup_init = false;

    /// Options used in the binding mappings for external textures
    ExternalTextureOptions external_texture_options = {};

    /// Options used to specify a mapping of binding points to indices into a UBO
    /// from which to load buffer sizes.
    ArrayLengthFromUniformOptions array_length_from_uniform = {};

    /// Options used in the bindings remapper
    BindingRemapperOptions binding_remapper_options = {};

    /// Interstage locations actually used as inputs in the next stage of the pipeline.
    /// This is potentially used for truncating unused interstage outputs at current shader stage.
    std::bitset<16> interstage_locations;

    /// Set to `true` to run the TruncateInterstageVariables transform.
    bool truncate_interstage_variables = false;

    /// Set to `true` to generate polyfill for `reflect` builtin for vec2<f32>
    bool polyfill_reflect_vec2_f32 = false;

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(disable_robustness,
                 root_constant_binding_point,
                 disable_workgroup_init,
                 external_texture_options,
                 array_length_from_uniform);
};

/// The result produced when generating HLSL.
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

    /// The generated HLSL.
    std::string hlsl = "";

    /// The list of entry points in the generated HLSL.
    std::vector<std::pair<std::string, ast::PipelineStage>> entry_points;

    /// Indices into the array_length_from_uniform binding that are statically
    /// used.
    std::unordered_set<uint32_t> used_array_length_from_uniform_indices;
};

/// Generate HLSL for a program, according to a set of configuration options.
/// The result will contain the HLSL, as well as success status and diagnostic
/// information.
/// @param program the program to translate to HLSL
/// @param options the configuration options to use when generating HLSL
/// @returns the resulting HLSL and supplementary information
Result Generate(const Program* program, const Options& options);

}  // namespace tint::writer::hlsl

#endif  // SRC_TINT_WRITER_HLSL_GENERATOR_H_
