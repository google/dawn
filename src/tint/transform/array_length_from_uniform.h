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

#ifndef SRC_TINT_TRANSFORM_ARRAY_LENGTH_FROM_UNIFORM_H_
#define SRC_TINT_TRANSFORM_ARRAY_LENGTH_FROM_UNIFORM_H_

#include <unordered_map>
#include <unordered_set>

#include "src/tint/sem/binding_point.h"
#include "src/tint/transform/transform.h"

// Forward declarations
namespace tint {
class CloneContext;
}  // namespace tint

namespace tint::transform {

/// ArrayLengthFromUniform is a transform that implements calls to arrayLength()
/// by calculating the length from the total size of the storage buffer, which
/// is received via a uniform buffer.
///
/// The generated uniform buffer will have the form:
/// ```
/// struct buffer_size_struct {
///  buffer_size : array<u32, 8>;
/// };
///
/// @group(0) @binding(30)
/// var<uniform> buffer_size_ubo : buffer_size_struct;
/// ```
/// The binding group and number used for this uniform buffer is provided via
/// the `Config` transform input. The `Config` struct also defines the mapping
/// from a storage buffer's `BindingPoint` to the array index that will be used
/// to get the size of that buffer.
///
/// This transform assumes that the `SimplifyPointers`
/// transforms have been run before it so that arguments to the arrayLength
/// builtin always have the form `&resource.array`.
///
/// @note Depends on the following transforms to have been run first:
/// * SimplifyPointers
class ArrayLengthFromUniform final : public utils::Castable<ArrayLengthFromUniform, Transform> {
  public:
    /// Constructor
    ArrayLengthFromUniform();
    /// Destructor
    ~ArrayLengthFromUniform() override;

    /// Configuration options for the ArrayLengthFromUniform transform.
    struct Config final : public utils::Castable<Data, transform::Data> {
        /// Constructor
        /// @param ubo_bp the binding point to use for the generated uniform buffer.
        explicit Config(sem::BindingPoint ubo_bp);

        /// Copy constructor
        Config(const Config&);

        /// Copy assignment
        /// @return this Config
        Config& operator=(const Config&);

        /// Destructor
        ~Config() override;

        /// The binding point to use for the generated uniform buffer.
        sem::BindingPoint ubo_binding;

        /// The mapping from binding point to the index for the buffer size lookup.
        std::unordered_map<sem::BindingPoint, uint32_t> bindpoint_to_size_index;
    };

    /// Information produced about what the transform did.
    /// If there were no calls to the arrayLength() builtin, then no Result will
    /// be emitted.
    struct Result final : public utils::Castable<Result, transform::Data> {
        /// Constructor
        /// @param used_size_indices Indices into the UBO that are statically used.
        explicit Result(std::unordered_set<uint32_t> used_size_indices);

        /// Copy constructor
        Result(const Result&);

        /// Destructor
        ~Result() override;

        /// Indices into the UBO that are statically used.
        std::unordered_set<uint32_t> used_size_indices;
    };

    /// @copydoc Transform::Apply
    ApplyResult Apply(const Program* program,
                      const DataMap& inputs,
                      DataMap& outputs) const override;

  private:
    struct State;
};

}  // namespace tint::transform

#endif  // SRC_TINT_TRANSFORM_ARRAY_LENGTH_FROM_UNIFORM_H_
