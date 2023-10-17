// Copyright 2022 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_MSL_WRITER_AST_RAISE_PACKED_VEC3_H_
#define SRC_TINT_LANG_MSL_WRITER_AST_RAISE_PACKED_VEC3_H_

#include "src/tint/lang/wgsl/ast/transform/transform.h"

namespace tint::msl::writer {

/// A transform to be used by the MSL backend which will:
/// * Replace `vec3<T>` types with an internal `__packed_vec3` type when they are used in
///   host-shareable address spaces.
/// * Wrap generated `__packed_vec3` types in a structure when they are used in arrays, so that we
///   ensure that the array has the correct element stride.
/// * Multi-version structures that contain `vec3<T>` types when they are used in host-shareable
///   memory, to avoid modifying uses in other address spaces.
/// * Rewrite matrix types that have three rows into arrays of column vectors.
/// * Insert calls to helper functions to convert expressions that use these types to or from the
///   regular vec3 types when accessing host-shareable memory.
/// * Cast all direct (not sub-accessed) loads of these packed vectors to the 'unpacked' vec3<T>
///   type before usage.
///
/// This transform is necessary in order to emit vec3 types with the correct size (so that scalars
/// can follow them in structures), and also to ensure that padding bytes are preserved when writing
/// to a vec3, an array of vec3 elements, or a matrix with vec3 column type.
///
/// @note Depends on the following transforms to have been run first:
/// * ExpandCompoundAssignment
class PackedVec3 final : public Castable<PackedVec3, ast::transform::Transform> {
  public:
    /// Constructor
    PackedVec3();
    /// Destructor
    ~PackedVec3() override;

    /// @copydoc ast::transform::Transform::Apply
    ApplyResult Apply(const Program& program,
                      const ast::transform::DataMap& inputs,
                      ast::transform::DataMap& outputs) const override;

  private:
    struct State;
};

}  // namespace tint::msl::writer

#endif  // SRC_TINT_LANG_MSL_WRITER_AST_RAISE_PACKED_VEC3_H_
