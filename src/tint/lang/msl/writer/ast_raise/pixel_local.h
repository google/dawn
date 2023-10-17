// Copyright 2023 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_MSL_WRITER_AST_RAISE_PIXEL_LOCAL_H_
#define SRC_TINT_LANG_MSL_WRITER_AST_RAISE_PIXEL_LOCAL_H_

#include <string>

#include "src/tint/lang/wgsl/ast/internal_attribute.h"
#include "src/tint/lang/wgsl/ast/transform/transform.h"
#include "src/tint/utils/containers/hashmap.h"

namespace tint::msl::writer {

/// PixelLocal transforms module-scope `var<pixel_local>`s and fragment entry point functions that
/// use them:
/// * `var<pixel_local>` will be transformed to `var<private>`.
/// * The entry point function will be wrapped with another function ('outer') that calls the
///  'inner' function.
/// * The outer function will have an additional parameter of the pixel local struct type, which is
///   copied to the module-scope var before calling the 'inner' function.
/// * The outer function will have a new struct return type which holds both the pixel local members
///   and the returned value(s) of the 'inner' function.
/// @note PixelLocal requires that the SingleEntryPoint transform has already been run
class PixelLocal final : public Castable<PixelLocal, ast::transform::Transform> {
  public:
    /// Transform configuration options
    struct Config final : public Castable<Config, ast::transform::Data> {
        /// Constructor
        Config();

        /// Copy Constructor
        Config(const Config&);

        /// Destructor
        ~Config() override;

        /// Index of pixel_local structure member index to attachment index
        Hashmap<uint32_t, uint32_t, 8> attachments;
    };

    /// Intrinsic is an InternalAttribute that's used to decorate a pixel local attachment
    /// parameter, return value or structure member.
    class Attachment final : public Castable<Attachment, ast::InternalAttribute> {
      public:
        /// Constructor
        /// @param pid the identifier of the program that owns this node
        /// @param nid the unique node identifier
        /// @param idx the attachment index
        Attachment(GenerationID pid, ast::NodeID nid, uint32_t idx);

        /// Destructor
        ~Attachment() override;

        /// @return a short description of the internal attribute which will be
        /// displayed as `@internal(<name>)`
        std::string InternalName() const override;

        /// Performs a deep clone of this object using the program::CloneContext `ctx`.
        /// @param ctx the clone context
        /// @return the newly cloned object
        const Attachment* Clone(ast::CloneContext& ctx) const override;

        /// The attachment index
        const uint32_t index;
    };

    /// Constructor
    PixelLocal();

    /// Destructor
    ~PixelLocal() override;

    /// @copydoc ast::transform::Transform::Apply
    ApplyResult Apply(const Program& program,
                      const ast::transform::DataMap& inputs,
                      ast::transform::DataMap& outputs) const override;

  private:
    struct State;
};

}  // namespace tint::msl::writer

#endif  // SRC_TINT_LANG_MSL_WRITER_AST_RAISE_PIXEL_LOCAL_H_
