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

#ifndef SRC_TINT_WRITER_MSL_GENERATOR_H_
#define SRC_TINT_WRITER_MSL_GENERATOR_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "src/tint/reflection.h"
#include "src/tint/writer/array_length_from_uniform_options.h"
#include "src/tint/writer/binding_remapper_options.h"
#include "src/tint/writer/external_texture_options.h"
#include "src/tint/writer/text.h"

// Forward declarations
namespace tint {
class Program;
}  // namespace tint

namespace tint::writer::msl {

/// Configuration options used for generating MSL.
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

    /// The index to use when generating a UBO to receive storage buffer sizes.
    /// Defaults to 30, which is the last valid buffer slot.
    uint32_t buffer_size_ubo_index = 30;

    /// The fixed sample mask to combine with fragment shader outputs.
    /// Defaults to 0xFFFFFFFF.
    uint32_t fixed_sample_mask = 0xFFFFFFFF;

    /// Set to `true` to generate a [[point_size]] attribute which is set to 1.0
    /// for all vertex shaders in the module.
    bool emit_vertex_point_size = false;

    /// Set to `true` to disable workgroup memory zero initialization
    bool disable_workgroup_init = false;

    /// Options used in the binding mappings for external textures
    ExternalTextureOptions external_texture_options = {};

    /// Options used to specify a mapping of binding points to indices into a UBO
    /// from which to load buffer sizes.
    ArrayLengthFromUniformOptions array_length_from_uniform = {};

    /// Options used in the bindings remapper
    BindingRemapperOptions binding_remapper_options = {};

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(disable_robustness,
                 buffer_size_ubo_index,
                 fixed_sample_mask,
                 emit_vertex_point_size,
                 disable_workgroup_init,
                 external_texture_options,
                 array_length_from_uniform);
};

/// The result produced when generating MSL.
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

    /// The generated MSL.
    std::string msl = "";

    /// True if the shader needs a UBO of buffer sizes.
    bool needs_storage_buffer_sizes = false;

    /// True if the generated shader uses the invariant attribute.
    bool has_invariant_attribute = false;

    /// A map from entry point name to a list of dynamic workgroup allocations.
    /// Each entry in the vector is the size of the workgroup allocation that
    /// should be created for that index.
    std::unordered_map<std::string, std::vector<uint32_t>> workgroup_allocations;

    /// Indices into the array_length_from_uniform binding that are statically
    /// used.
    std::unordered_set<uint32_t> used_array_length_from_uniform_indices;
};

/// Generate MSL for a program, according to a set of configuration options. The
/// result will contain the MSL, as well as success status and diagnostic
/// information.
/// @param program the program to translate to MSL
/// @param options the configuration options to use when generating MSL
/// @returns the resulting MSL and supplementary information
Result Generate(const Program* program, const Options& options);

}  // namespace tint::writer::msl

#endif  // SRC_TINT_WRITER_MSL_GENERATOR_H_
