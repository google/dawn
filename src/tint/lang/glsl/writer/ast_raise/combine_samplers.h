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

#ifndef SRC_TINT_LANG_GLSL_WRITER_AST_RAISE_COMBINE_SAMPLERS_H_
#define SRC_TINT_LANG_GLSL_WRITER_AST_RAISE_COMBINE_SAMPLERS_H_

#include <string>
#include <unordered_map>

#include "src/tint/lang/wgsl/ast/transform/transform.h"
#include "src/tint/lang/wgsl/sem/sampler_texture_pair.h"

namespace tint::glsl::writer {

/// This transform converts all separate texture/sampler refences in a
/// program into combined texture/samplers. This is required for GLSL,
/// which does not support separate texture/samplers.
///
/// It utilizes the texture/sampler information collected by the
/// Resolver and stored on each sem::Function. For each function, all
/// separate texture/sampler parameters in the function signature are
/// removed. For each unique pair, if both texture and sampler are
/// global variables, the function passes the corresponding combined
/// global stored in global_combined_texture_samplers_ at the call
/// site. Otherwise, either the texture or sampler must be a function
/// parameter. In this case, a new parameter is added to the function
/// signature. All separate texture/sampler parameters are removed.
///
/// All texture builtin callsites are modified to pass the combined
/// texture/sampler as the first argument, and separate texture/sampler
/// arguments are removed.
///
/// Note that the sampler may be null, indicating that only a texture
/// reference was required (e.g., textureLoad). In this case, a
/// placeholder global sampler is used at the AST level. This will be
/// combined with the original texture to give a combined global, and
/// the placeholder removed (ignored) by the GLSL writer.
///
/// Note that the combined samplers are actually represented by a
/// Texture node at the AST level, since this contains all the
/// information needed to represent a combined sampler in GLSL
/// (dimensionality, component type, etc). The GLSL writer outputs such
/// (Tint) Textures as (GLSL) Samplers.
class CombineSamplers final : public Castable<CombineSamplers, ast::transform::Transform> {
  public:
    /// A pair of binding points.
    using SamplerTexturePair = sem::SamplerTexturePair;

    /// A map from a sampler/texture pair to a named global.
    using BindingMap = std::unordered_map<SamplerTexturePair, std::string>;

    /// The client-provided mapping from separate texture and sampler binding
    /// points to combined sampler binding point.
    struct BindingInfo final : public Castable<BindingInfo, ast::transform::Data> {
        /// Constructor
        /// @param map the map of all (texture, sampler) -> (combined) pairs
        /// @param placeholder the binding point to use for placeholder samplers.
        BindingInfo(const BindingMap& map, const BindingPoint& placeholder);

        /// Copy constructor
        /// @param other the other BindingInfo to copy
        BindingInfo(const BindingInfo& other);

        /// Destructor
        ~BindingInfo() override;

        /// A map of bindings from (texture, sampler) -> combined sampler.
        BindingMap binding_map;

        /// The binding point to use for placeholder samplers.
        BindingPoint placeholder_binding_point;
    };

    /// Constructor
    CombineSamplers();

    /// Destructor
    ~CombineSamplers() override;

    /// @copydoc ast::transform::Transform::Apply
    ApplyResult Apply(const Program& program,
                      const ast::transform::DataMap& inputs,
                      ast::transform::DataMap& outputs) const override;

  private:
    struct State;
};

}  // namespace tint::glsl::writer

#endif  // SRC_TINT_LANG_GLSL_WRITER_AST_RAISE_COMBINE_SAMPLERS_H_
