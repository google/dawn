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

#ifndef SRC_TINT_TRANSFORM_FIRST_INDEX_OFFSET_H_
#define SRC_TINT_TRANSFORM_FIRST_INDEX_OFFSET_H_

#include "src/tint/transform/transform.h"

namespace tint::transform {

/// Adds firstVertex/Instance (injected via root constants) to
/// vertex/instance index builtins.
///
/// This transform assumes that Name transform has been run before.
///
/// Unlike other APIs, D3D always starts vertex and instance numbering at 0,
/// regardless of the firstVertex/Instance value specified. This transformer
/// adds the value of firstVertex/Instance to each builtin. This action is
/// performed by adding a new constant equal to original builtin +
/// firstVertex/Instance to each function that references one of these builtins.
///
/// Note that D3D does not have any semantics for firstVertex/Instance.
/// Therefore, these values must by passed to the shader.
///
/// Before:
/// ```
///   @builtin(vertex_index) var<in> vert_idx : u32;
///   fn func() -> u32 {
///     return vert_idx;
///   }
/// ```
///
/// After:
/// ```
///   struct TintFirstIndexOffsetData {
///     tint_first_vertex_index : u32;
///     tint_first_instance_index : u32;
///   };
///   @builtin(vertex_index) var<in> tint_first_index_offset_vert_idx : u32;
///   @binding(N) @group(M) var<uniform> tint_first_index_data :
///                                                    TintFirstIndexOffsetData;
///   fn func() -> u32 {
///     const vert_idx = (tint_first_index_offset_vert_idx +
///                       tint_first_index_data.tint_first_vertex_index);
///     return vert_idx;
///   }
/// ```
///
class FirstIndexOffset final : public utils::Castable<FirstIndexOffset, Transform> {
  public:
    /// BindingPoint is consumed by the FirstIndexOffset transform.
    /// BindingPoint specifies the binding point of the first index uniform
    /// buffer.
    struct BindingPoint final : public utils::Castable<BindingPoint, transform::Data> {
        /// Constructor
        BindingPoint();

        /// Constructor
        /// @param b the binding index
        /// @param g the binding group
        BindingPoint(uint32_t b, uint32_t g);

        /// Destructor
        ~BindingPoint() override;

        /// `@binding()` for the first vertex / first instance uniform buffer
        uint32_t binding = 0;
        /// `@group()` for the first vertex / first instance uniform buffer
        uint32_t group = 0;
    };

    /// Data is outputted by the FirstIndexOffset transform.
    /// Data holds information about shader usage and constant buffer offsets.
    struct Data final : public utils::Castable<Data, transform::Data> {
        /// Constructor
        /// @param has_vtx_or_inst_index True if the shader uses vertex_index or
        /// instance_index
        explicit Data(bool has_vtx_or_inst_index);

        /// Copy constructor
        Data(const Data&);

        /// Destructor
        ~Data() override;

        /// True if the shader uses vertex_index
        const bool has_vertex_or_instance_index;
    };

    /// Constructor
    FirstIndexOffset();
    /// Destructor
    ~FirstIndexOffset() override;

    /// @copydoc Transform::Apply
    ApplyResult Apply(const Program* program,
                      const DataMap& inputs,
                      DataMap& outputs) const override;

  private:
    uint32_t binding_ = 0;
    uint32_t group_ = 0;
};

}  // namespace tint::transform

#endif  // SRC_TINT_TRANSFORM_FIRST_INDEX_OFFSET_H_
