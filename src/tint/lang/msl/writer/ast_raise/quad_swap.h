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

#ifndef SRC_TINT_LANG_MSL_WRITER_AST_RAISE_QUAD_SWAP_H_
#define SRC_TINT_LANG_MSL_WRITER_AST_RAISE_QUAD_SWAP_H_

#include <string>

#include "src/tint/lang/wgsl/ast/internal_attribute.h"
#include "src/tint/lang/wgsl/ast/transform/transform.h"

namespace tint::msl::writer {

/// QuadSwap is a transform that replaces calls to `quadSwap{X, Y, Diagonal}()` with an
/// implementation that uses MSL's `quad_shuffle()` using the following mapping:
/// +------------------+------------------------------------------------------------------+
/// |       WGSL       |                               MSL                                |
/// +------------------+------------------------------------------------------------------+
/// | quadSwapX        | quad_shuffle with quad_lane_id=thread_index_in_quad_group ^ 0b1  |
/// | quadSwapY        | quad_shuffle with quad_lane_id=thread_index_in_quad_group ^ 0b10 |
/// | quadSwapDiagonal | quad_shuffle with quad_lane_id=thread_index_in_quad_group ^ 0b11 |
/// +------------------+------------------------------------------------------------------+
///
/// @note Depends on the following transforms to have been run first:
/// * CanonicalizeEntryPointIO
class QuadSwap final : public Castable<QuadSwap, ast::transform::Transform> {
  public:
    /// Constructor
    QuadSwap();

    /// Destructor
    ~QuadSwap() override;

    /// @copydoc ast::transform::Transform::Apply
    ApplyResult Apply(const Program& program,
                      const ast::transform::DataMap& inputs,
                      ast::transform::DataMap& outputs) const override;

    /// QuadShuffle is an InternalAttribute that is used to decorate a stub function so
    /// that the MSL backend transforms this into calls to the `quad_shuffle` function.
    class QuadShuffle final : public Castable<QuadShuffle, ast::InternalAttribute> {
      public:
        /// Constructor
        /// @param pid the identifier of the program that owns this node
        /// @param nid the unique node identifier
        QuadShuffle(GenerationID pid, ast::NodeID nid) : Base(pid, nid, Empty) {}

        /// Destructor
        ~QuadShuffle() override;

        /// @copydoc ast::InternalAttribute::InternalName
        std::string InternalName() const override { return "quad_shuffle"; }

        /// Performs a deep clone of this object using the program::CloneContext `ctx`.
        /// @param ctx the clone context
        /// @return the newly cloned object
        const QuadShuffle* Clone(ast::CloneContext& ctx) const override;
    };

    /// ThreadIndexInQuadgroup is an InternalAttribute that is used to decorate an entrypoint
    /// parameter so that the MSL backend transforms this into a `[[thread_index_in_quadgroup]]`
    /// attribute.
    class ThreadIndexInQuadgroup final
        : public Castable<ThreadIndexInQuadgroup, ast::InternalAttribute> {
      public:
        /// Constructor
        /// @param pid the identifier of the program that owns this node
        /// @param nid the unique node identifier
        ThreadIndexInQuadgroup(GenerationID pid, ast::NodeID nid) : Base(pid, nid, Empty) {}

        /// Destructor
        ~ThreadIndexInQuadgroup() override;

        /// @copydoc ast::InternalAttribute::InternalName
        std::string InternalName() const override { return "thread_index_in_quadgroup"; }

        /// Performs a deep clone of this object using the program::CloneContext `ctx`.
        /// @param ctx the clone context
        /// @return the newly cloned object
        const ThreadIndexInQuadgroup* Clone(ast::CloneContext& ctx) const override;
    };

  private:
    struct State;
};

}  // namespace tint::msl::writer

#endif  // SRC_TINT_LANG_MSL_WRITER_AST_RAISE_QUAD_SWAP_H_
