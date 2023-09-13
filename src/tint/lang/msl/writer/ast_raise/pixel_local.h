// Copyright 2023 The Tint Authors.
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
    ApplyResult Apply(const Program* program,
                      const ast::transform::DataMap& inputs,
                      ast::transform::DataMap& outputs) const override;

  private:
    struct State;
};

}  // namespace tint::msl::writer

#endif  // SRC_TINT_LANG_MSL_WRITER_AST_RAISE_PIXEL_LOCAL_H_
