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

#include "src/tint/lang/glsl/writer/common/option_helpers.h"

#include <utility>

#include "src/tint/utils/containers/hashmap.h"
#include "src/tint/utils/diagnostic/diagnostic.h"

namespace tint::glsl::writer {

namespace {

enum class BindingType : uint8_t {
    kUniform,
    kStorage,
    kSampler,
    kTexture,
    kStorageTexture,
    kExternalTexture
};

std::string_view ToString(BindingType value) {
    switch (value) {
        case BindingType::kUniform:
            return "uniform";
        case BindingType::kStorage:
            return "storage";
        case BindingType::kSampler:
            return "sampler";
        case BindingType::kTexture:
            return "texture";
        case BindingType::kStorageTexture:
            return "storage_texture";
        case BindingType::kExternalTexture:
            return "external_texture";
    }
    TINT_ICE() << "Unknown enum passed to ToString(::BindingType)";
}

}  // namespace

Result<SuccessType> ValidateBindingOptions(const Options& options) {
    diag::List diagnostics;

    tint::Hashmap<tint::BindingPoint, binding::BindingInfo, 8> seen_wgsl_bindings{};
    tint::Hashmap<binding::BindingInfo, tint::BindingPoint, 8> seen_glsl_texture_bindings{};
    tint::Hashmap<binding::BindingInfo, tint::BindingPoint, 8> seen_glsl_sampler_bindings{};
    tint::Hashmap<binding::BindingInfo, tint::BindingPoint, 8> seen_glsl_other_bindings{};
    // Both wgsl_seen and glsl_seen check to see if the pair of [src, dst] are unique. If
    // we have multiple entries that map the same [src, dst] pair, that's fine. We treat it as valid
    // as it's possible for multiple entry points to use the remapper at the same time. If the pair
    // doesn't match, then we report an error about a duplicate binding point.
    // For glsl_seen it is also valid for a texture and a sampler have the same GLSL side binding
    // point.
    auto wgsl_seen = [&diagnostics, &seen_wgsl_bindings](const tint::BindingPoint& src,
                                                         const binding::BindingInfo& dst) -> bool {
        if (auto binding = seen_wgsl_bindings.Add(src, dst); binding.value != dst) {
            diagnostics.AddError(Source{}) << "found duplicate WGSL binding point: " << src;
            return true;
        }
        return false;
    };

    auto glsl_seen = [&diagnostics, &seen_glsl_texture_bindings, &seen_glsl_sampler_bindings,
                      &seen_glsl_other_bindings](const binding::BindingInfo& src,
                                                 const tint::BindingPoint& dst,
                                                 BindingType type) -> bool {
        auto disallowed_duplicate =
            [&diagnostics, &src,
             &dst](const tint::Hashmap<binding::BindingInfo, tint::BindingPoint, 8>& seen_bindings)
            -> bool {
            if (auto binding = seen_bindings.Get(src)) {
                if (*binding != dst) {
                    diagnostics.AddError(Source{})
                        << "found duplicate GLSL binding point: [binding: " << src.binding << "]";
                    return true;
                }
            }
            return false;
        };

        switch (type) {
            case BindingType::kTexture:
                if (disallowed_duplicate(seen_glsl_texture_bindings) ||
                    disallowed_duplicate(seen_glsl_other_bindings)) {
                    return true;
                }
                seen_glsl_texture_bindings.Add(src, dst);
                break;
            case BindingType::kSampler:
                if (disallowed_duplicate(seen_glsl_sampler_bindings) ||
                    disallowed_duplicate(seen_glsl_other_bindings)) {
                    return true;
                }
                seen_glsl_sampler_bindings.Add(src, dst);
                break;
            default:
                if (disallowed_duplicate(seen_glsl_sampler_bindings) ||
                    disallowed_duplicate(seen_glsl_texture_bindings) ||
                    disallowed_duplicate(seen_glsl_other_bindings)) {
                    return true;
                }
                seen_glsl_other_bindings.Add(src, dst);
                break;
        }
        return false;
    };

    auto valid = [&wgsl_seen, &glsl_seen, &diagnostics](const auto& hsh, BindingType type) -> bool {
        for (const auto& it : hsh) {
            const auto& src_binding = it.first;
            const auto& dst_binding = it.second;

            if (wgsl_seen(src_binding, dst_binding) || glsl_seen(dst_binding, src_binding, type)) {
                diagnostics.AddNote(Source{}) << "when processing " << ToString(type);
                return false;
            }
        }
        return true;
    };

    if (!valid(options.bindings.uniform, BindingType::kUniform)) {
        return Failure{diagnostics.Str()};
    }
    if (!valid(options.bindings.storage, BindingType::kStorage)) {
        return Failure{diagnostics.Str()};
    }
    if (!valid(options.bindings.sampler, BindingType::kSampler)) {
        return Failure{diagnostics.Str()};
    }
    if (!valid(options.bindings.texture, BindingType::kTexture)) {
        return Failure{diagnostics.Str()};
    }
    if (!valid(options.bindings.storage_texture, BindingType::kStorageTexture)) {
        return Failure{diagnostics.Str()};
    }

    for (const auto& it : options.bindings.external_texture) {
        const auto& src_binding = it.first;
        const auto& plane0 = it.second.plane0;
        const auto& plane1 = it.second.plane1;
        const auto& metadata = it.second.metadata;

        // Validate with the actual source regardless of what the remapper will do
        if (wgsl_seen(src_binding, plane0)) {
            diagnostics.AddNote(Source{}) << "when processing external_texture";
            return Failure{diagnostics.Str()};
        }

        if (glsl_seen(plane0, src_binding, BindingType::kExternalTexture)) {
            return Failure{diagnostics.Str()};
        }
        if (glsl_seen(plane1, src_binding, BindingType::kExternalTexture)) {
            return Failure{diagnostics.Str()};
        }
        if (glsl_seen(metadata, src_binding, BindingType::kExternalTexture)) {
            return Failure{diagnostics.Str()};
        }
    }

    return Success;
}

// The remapped binding data and external texture data need to coordinate in order to put
// things in the correct place when we're done. When the data comes in we have a list of all WGSL
// origin (group,binding) pairs to GLSL (binding) in the `uniform`, `storage`, `texture`, and
// `sampler` arrays.
void PopulateBindingInfo(const Options& options,
                         RemapperData& remapper_data,
                         tint::transform::multiplanar::BindingsMap& multiplanar_map) {
    auto create_remappings = [&remapper_data](const auto& hsh) {
        for (const auto& it : hsh) {
            const BindingPoint& src_binding_point = it.first;
            const binding::Uniform& dst_binding_point = it.second;

            // Bindings which go to the same slot in GLSL do not need to be re-bound.
            if (src_binding_point.group == 0 &&
                src_binding_point.binding == dst_binding_point.binding) {
                continue;
            }

            remapper_data.emplace(src_binding_point, BindingPoint{0, dst_binding_point.binding});
        }
    };

    create_remappings(options.bindings.uniform);
    create_remappings(options.bindings.storage);
    create_remappings(options.bindings.texture);
    create_remappings(options.bindings.storage_texture);
    create_remappings(options.bindings.sampler);

    // External textures are re-bound to their plane0 location
    for (const auto& it : options.bindings.external_texture) {
        const BindingPoint& src_binding_point = it.first;

        const binding::BindingInfo& plane0 = it.second.plane0;
        const binding::BindingInfo& plane1 = it.second.plane1;
        const binding::BindingInfo& metadata = it.second.metadata;

        const BindingPoint plane0_binding_point{0, plane0.binding};
        const BindingPoint plane1_binding_point{0, plane1.binding};
        const BindingPoint metadata_binding_point{0, metadata.binding};

        // Use the re-bound glsl plane0 value for the lookup key.
        multiplanar_map.emplace(BindingPoint{0, plane0_binding_point.binding},
                                tint::transform::multiplanar::BindingPoints{
                                    plane1_binding_point, metadata_binding_point});

        // Bindings which go to the same slot in GLSL do not need to be re-bound.
        if (src_binding_point == plane0_binding_point) {
            continue;
        }

        remapper_data.emplace(src_binding_point, plane0_binding_point);
    }

    // Update the non-plane1 bindings in the combined texture sampler info to be the
    // remapped bindings.
    for (const auto& it : options.bindings.sampler_texture_to_name) {
        auto pair = it.first;
        auto name = it.second;

        // Move the non-external textures to the new binding points
        if (!pair.is_external_plane1) {
            pair.texture.group = 0;
            pair.sampler.group = 0;

            auto tex = options.bindings.texture.find(pair.texture);
            if (tex != options.bindings.texture.end()) {
                pair.texture.binding = tex->second.binding;
            }

            auto samp = options.bindings.sampler.find(pair.sampler);
            if (samp != options.bindings.sampler.end()) {
                pair.sampler.binding = samp->second.binding;
            }
        }
    }
}

}  // namespace tint::glsl::writer
