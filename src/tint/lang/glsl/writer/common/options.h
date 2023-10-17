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

#ifndef SRC_TINT_LANG_GLSL_WRITER_COMMON_OPTIONS_H_
#define SRC_TINT_LANG_GLSL_WRITER_COMMON_OPTIONS_H_

#include <optional>
#include <string>
#include <unordered_map>

#include "src/tint/api/options/binding_remapper.h"
#include "src/tint/api/options/external_texture.h"
#include "src/tint/api/options/texture_builtins_from_uniform.h"
#include "src/tint/lang/core/access.h"
#include "src/tint/lang/glsl/writer/common/version.h"
#include "src/tint/lang/wgsl/sem/sampler_texture_pair.h"

namespace tint::glsl::writer {

using SamplerTexturePair = sem::SamplerTexturePair;
using BindingMap = std::unordered_map<SamplerTexturePair, std::string>;

/// Configuration options used for generating GLSL.
struct Options {
    /// Constructor
    Options();

    /// Destructor
    ~Options();

    /// Copy constructor
    Options(const Options&);

    /// Set to `true` to disable software robustness that prevents out-of-bounds accesses.
    bool disable_robustness = false;

    /// Set to `true` to disable workgroup memory zero initialization
    bool disable_workgroup_init = false;

    /// Set to `true` to generate GLSL via the Tint IR instead of from the AST.
    bool use_tint_ir = false;

    /// The GLSL version to emit
    Version version;

    /// A map of SamplerTexturePair to combined sampler names for the
    /// CombineSamplers transform
    BindingMap binding_map;

    /// The binding point to use for placeholder samplers.
    BindingPoint placeholder_binding_point;

    /// Options used in the bindings remapper
    BindingRemapperOptions binding_remapper_options = {};

    /// Options used in the binding mappings for external textures
    ExternalTextureOptions external_texture_options = {};

    /// Options used to map WGSL textureNumLevels/textureNumSamples builtins to internal uniform
    /// buffer values. If not specified, emits corresponding GLSL builtins
    /// textureQueryLevels/textureSamples directly.
    std::optional<TextureBuiltinsFromUniformOptions> texture_builtins_from_uniform = std::nullopt;

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(disable_robustness,
                 disable_workgroup_init,
                 version,
                 binding_map,
                 placeholder_binding_point,
                 binding_remapper_options,
                 external_texture_options,
                 texture_builtins_from_uniform);
};

}  // namespace tint::glsl::writer

#endif  // SRC_TINT_LANG_GLSL_WRITER_COMMON_OPTIONS_H_
