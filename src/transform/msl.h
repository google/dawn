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

#ifndef SRC_TRANSFORM_MSL_H_
#define SRC_TRANSFORM_MSL_H_

#include "src/transform/transform.h"

namespace tint {
namespace transform {

/// Msl is a transform used to sanitize a Program for use with the Msl writer.
/// Passing a non-sanitized Program to the Msl writer will result in undefined
/// behavior.
class Msl : public Castable<Msl, Transform> {
 public:
  /// The default buffer slot to use for the storage buffer size buffer.
  const uint32_t kDefaultBufferSizeUniformIndex = 30;

  /// Configuration options for the Msl sanitizer transform.
  struct Config : public Castable<Data, transform::Data> {
    /// Constructor
    /// @param buffer_size_ubo_idx the index to use for the buffer size UBO
    /// @param sample_mask the fixed sample mask to use for fragment shaders
    /// @param emit_point_size `true` to emit a vertex point size builtin
    /// @param disable_workgroup_init `true` to disable workgroup memory zero
    ///        initialization
    Config(uint32_t buffer_size_ubo_idx,
           uint32_t sample_mask = 0xFFFFFFFF,
           bool emit_point_size = false,
           bool disable_workgroup_init = false);

    /// Copy constructor
    Config(const Config&);

    /// Destructor
    ~Config() override;

    /// The index to use when generating a UBO to receive storage buffer sizes.
    uint32_t buffer_size_ubo_index = 0;

    /// The fixed sample mask to combine with fragment shader outputs.
    uint32_t fixed_sample_mask = 0xFFFFFFFF;

    /// Set to `true` to generate a [[point_size]] attribute which is set to 1.0
    /// for all vertex shaders in the module.
    bool emit_vertex_point_size = false;

    /// Set to `true` to disable workgroup memory zero initialization
    bool disable_workgroup_init = false;
  };

  /// Information produced by the sanitizer that users may need to act on.
  struct Result : public Castable<Result, transform::Data> {
    /// Constructor
    /// @param needs_buffer_sizes True if the shader needs a UBO of buffer sizes
    explicit Result(bool needs_buffer_sizes);

    /// Copy constructor
    Result(const Result&);

    /// Destructor
    ~Result() override;

    /// True if the shader needs a UBO of buffer sizes.
    bool const needs_storage_buffer_sizes;
  };

  /// Constructor
  Msl();
  ~Msl() override;

  /// Runs the transform on `program`, returning the transformation result.
  /// @param program the source program to transform
  /// @param data optional extra transform-specific input data
  /// @returns the transformation result
  Output Run(const Program* program, const DataMap& data = {}) override;

 private:
  /// Pushes module-scope variables with certain storage classes into the entry
  /// point function, and passes them as function parameters to any functions
  /// that need them.
  void HandleModuleScopeVariables(CloneContext& ctx) const;
};

}  // namespace transform
}  // namespace tint

#endif  // SRC_TRANSFORM_MSL_H_
