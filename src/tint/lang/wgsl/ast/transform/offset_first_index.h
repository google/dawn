// Copyright 2024 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_TINT_LANG_WGSL_AST_TRANSFORM_OFFSET_FIRST_INDEX_H_
#define SRC_TINT_LANG_WGSL_AST_TRANSFORM_OFFSET_FIRST_INDEX_H_

#include "src/tint/lang/wgsl/ast/transform/transform.h"
#include "src/tint/utils/reflection/reflection.h"

namespace tint::ast::transform {

/// Adds firstVertex/Instance (injected via push constants) to
/// vertex/instance index builtins.
///
/// This transform assumes that the Name and AddBlockAttribute transforms have been run before.
///
/// Some shading languages start vertex and instance numbering at 0,
/// regardless of the firstVertex/firstInstance value specified. This transform
/// adds the value of firstVertex/firstInstance to each builtin. This action is
/// performed by adding a new push constant equal to original builtin +
/// firstVertex/firstInstance to each function that references one of
/// these builtins.
///
/// For D3D, this affects both firstVertex and firstInstance. For OpenGL,
/// it applies to only firstInstance. For this reason, the first_vertex_offset
/// and first_instance_offset are optional, where std::nullopt indicates that
/// no substitution is to be performed.
///
/// Before:
/// ```
///   @builtin(vertex_index) var<in> vert_idx : u32;
///   @builtin(instance_index) var<in> inst_idx : u32;
///   fn func() -> u32 {
///     return vert_idx * inst_idx;
///   }
/// ```
///
/// After:
/// ```
/// struct PushConstants {
///   first_vertex : u32,
///   first_instance : u32,
/// }
///
/// var<push_constant> push_constants : PushConstants;
///
///   @builtin(vertex_index) var<in> vert_idx : u32;
///   @builtin(instance_index) var<in> inst_idx : u32;
///   fn func() -> u32 {
///     return (vert_idx + push_constants.first_vertex) * (inst_idx +
///     push_constants.first_instance);
///   }
/// ```
///
class OffsetFirstIndex final : public Castable<OffsetFirstIndex, Transform> {
  public:
    /// Transform configuration options
    struct Config final : public Castable<Config, ast::transform::Data> {
        /// Constructor
        Config();

        /// Constructor
        /// @param first_vertex_off Offset of the firstVertex push constant
        /// @param first_instance_off Location of the firstInstance push constant
        Config(std::optional<int32_t> first_vertex_off, std::optional<int32_t> first_instance_off);

        /// Destructor
        ~Config() override;

        /// Offset of the firstVertex push constant
        std::optional<uint32_t> first_vertex_offset;

        /// Offset of the firstInstance push constant
        std::optional<uint32_t> first_instance_offset;

        /// Reflection for this struct
        TINT_REFLECT(Config, first_vertex_offset, first_instance_offset);
    };

    /// Constructor
    OffsetFirstIndex();

    /// Destructor
    ~OffsetFirstIndex() override;

    /// @copydoc Transform::Apply
    ApplyResult Apply(const Program& program,
                      const DataMap& inputs,
                      DataMap& outputs) const override;
};

}  // namespace tint::ast::transform

#endif  // SRC_TINT_LANG_WGSL_AST_TRANSFORM_OFFSET_FIRST_INDEX_H_
