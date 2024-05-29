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
#include "src/tint/api/options/depth_range_offsets.h"
#include "src/tint/api/options/external_texture.h"
#include "src/tint/api/options/texture_builtins_from_uniform.h"
#include "src/tint/lang/glsl/writer/common/version.h"
#include "src/tint/lang/wgsl/ast/transform/transform.h"

namespace tint::glsl::writer::binding {

/// A combined texture/sampler pair
// Note, these are the WGSL binding points that are used to create the combined samplers
struct CombinedTextureSamplerPair {
    /// The WGSL texture binding
    BindingPoint texture = {};
    /// The WGSL sampler binding
    BindingPoint sampler = {};

    /// Equality operator
    /// @param rhs the CombinedTextureSamplerPair to compare against
    /// @returns true if this CombinedTextureSamplerPair is equal to `rhs`
    inline bool operator==(const CombinedTextureSamplerPair& rhs) const {
        return texture == rhs.texture && sampler == rhs.sampler;
    }

    /// Less then operator
    /// @param rhs the CombinedTextureSamplerPair to compare against
    /// @returns if this is less then rhs
    inline bool operator<(const CombinedTextureSamplerPair& rhs) const {
        if (texture < rhs.texture) {
            return true;
        }
        if (texture == rhs.texture) {
            return sampler < rhs.sampler;
        }
        return false;
    }

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(CombinedTextureSamplerPair, texture, sampler);
};

}  // namespace tint::glsl::writer::binding

namespace std {
/// Custom std::hash specialization for tint::glsl::writer::binding::CombinedTextureSamplerPair
template <>
class hash<tint::glsl::writer::binding::CombinedTextureSamplerPair> {
  public:
    /// @param n the combined sampler texture pair
    /// @return the hash value
    inline std::size_t operator()(
        const tint::glsl::writer::binding::CombinedTextureSamplerPair& n) const {
        return tint::Hash(n.texture, n.sampler);
    }
};

}  // namespace std

namespace tint::glsl::writer {

using CombinedTextureSamplerInfo =
    std::unordered_map<binding::CombinedTextureSamplerPair, std::string>;

struct CombineSamplersInfo final
    : public Castable<CombineSamplersInfo, tint::ast::transform::Data> {
    /// Constructor
    CombineSamplersInfo();

    /// Constructor
    /// @param map the texture/sampler to name map
    /// @param placeholder the sampler placeholder binding point
    CombineSamplersInfo(CombinedTextureSamplerInfo map, BindingPoint placeholder);

    CombineSamplersInfo(const CombineSamplersInfo&) = default;
    CombineSamplersInfo(CombineSamplersInfo&&) = default;

    /// Destructor
    ~CombineSamplersInfo() override;

    CombineSamplersInfo& operator=(const CombineSamplersInfo&) = default;
    CombineSamplersInfo& operator=(CombineSamplersInfo&&) = default;

    /// A map of SamplerTexturePair to combined sampler names for the
    /// CombineSamplers transform
    CombinedTextureSamplerInfo sampler_texture_to_name;

    /// The binding point to use for placeholder samplers.
    BindingPoint placeholder_sampler_binding;

    TINT_REFLECT(CombineSamplersInfo, sampler_texture_to_name, placeholder_sampler_binding);
};

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

    /// Set to `true` to disable the polyfills on integer division and modulo.
    bool disable_polyfill_integer_div_mod = false;

    /// The GLSL version to emit
    Version version;

    /// Combine Samplers transform information
    CombineSamplersInfo combined_samplers_info = {};

    /// Options used in the bindings remapper
    BindingRemapperOptions binding_remapper_options = {};

    /// Options used in the binding mappings for external textures
    ExternalTextureOptions external_texture_options = {};

    /// Offset of the firstVertex push constant.
    std::optional<int32_t> first_vertex_offset;

    /// Offset of the firstInstance push constant.
    std::optional<int32_t> first_instance_offset;

    /// Offsets of the minDepth and maxDepth push constants.
    std::optional<DepthRangeOffsets> depth_range_offsets;

    /// Options used to map WGSL textureNumLevels/textureNumSamples builtins to internal uniform
    /// buffer values. If not specified, emits corresponding GLSL builtins
    /// textureQueryLevels/textureSamples directly.
    TextureBuiltinsFromUniformOptions texture_builtins_from_uniform = {};

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(Options,
                 disable_robustness,
                 disable_workgroup_init,
                 disable_polyfill_integer_div_mod,
                 version,
                 combined_samplers_info,
                 binding_remapper_options,
                 external_texture_options,
                 first_vertex_offset,
                 first_instance_offset,
                 depth_range_offsets,
                 texture_builtins_from_uniform);
};

}  // namespace tint::glsl::writer

#endif  // SRC_TINT_LANG_GLSL_WRITER_COMMON_OPTIONS_H_
