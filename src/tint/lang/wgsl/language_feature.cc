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

////////////////////////////////////////////////////////////////////////////////
// File generated by 'tools/src/cmd/gen' using the template:
//   src/tint/lang/wgsl/language_feature.cc.tmpl
//
// To regenerate run: './tools/run gen'
//
//                       Do not modify this file directly
////////////////////////////////////////////////////////////////////////////////

#include "src/tint/lang/wgsl/language_feature.h"

namespace tint::wgsl {

/// ParseLanguageFeature parses a LanguageFeature from a string.
/// @param str the string to parse
/// @returns the parsed enum, or LanguageFeature::kUndefined if the string could not be parsed.
LanguageFeature ParseLanguageFeature(std::string_view str) {
    if (str == "chromium_testing_experimental") {
        return LanguageFeature::kChromiumTestingExperimental;
    }
    if (str == "chromium_testing_shipped") {
        return LanguageFeature::kChromiumTestingShipped;
    }
    if (str == "chromium_testing_shipped_with_killswitch") {
        return LanguageFeature::kChromiumTestingShippedWithKillswitch;
    }
    if (str == "chromium_testing_unimplemented") {
        return LanguageFeature::kChromiumTestingUnimplemented;
    }
    if (str == "chromium_testing_unsafe_experimental") {
        return LanguageFeature::kChromiumTestingUnsafeExperimental;
    }
    if (str == "packed_4x8_integer_dot_product") {
        return LanguageFeature::kPacked4X8IntegerDotProduct;
    }
    if (str == "pointer_composite_access") {
        return LanguageFeature::kPointerCompositeAccess;
    }
    if (str == "readonly_and_readwrite_storage_textures") {
        return LanguageFeature::kReadonlyAndReadwriteStorageTextures;
    }
    if (str == "sized_binding_array") {
        return LanguageFeature::kSizedBindingArray;
    }
    if (str == "texel_buffers") {
        return LanguageFeature::kTexelBuffers;
    }
    if (str == "texture_sample_level_1d") {
        return LanguageFeature::kTextureSampleLevel1D;
    }
    if (str == "unrestricted_pointer_parameters") {
        return LanguageFeature::kUnrestrictedPointerParameters;
    }
    return LanguageFeature::kUndefined;
}

std::string_view ToString(LanguageFeature value) {
    switch (value) {
        case LanguageFeature::kUndefined:
            return "undefined";
        case LanguageFeature::kChromiumTestingExperimental:
            return "chromium_testing_experimental";
        case LanguageFeature::kChromiumTestingShipped:
            return "chromium_testing_shipped";
        case LanguageFeature::kChromiumTestingShippedWithKillswitch:
            return "chromium_testing_shipped_with_killswitch";
        case LanguageFeature::kChromiumTestingUnimplemented:
            return "chromium_testing_unimplemented";
        case LanguageFeature::kChromiumTestingUnsafeExperimental:
            return "chromium_testing_unsafe_experimental";
        case LanguageFeature::kPacked4X8IntegerDotProduct:
            return "packed_4x8_integer_dot_product";
        case LanguageFeature::kPointerCompositeAccess:
            return "pointer_composite_access";
        case LanguageFeature::kReadonlyAndReadwriteStorageTextures:
            return "readonly_and_readwrite_storage_textures";
        case LanguageFeature::kSizedBindingArray:
            return "sized_binding_array";
        case LanguageFeature::kTexelBuffers:
            return "texel_buffers";
        case LanguageFeature::kTextureSampleLevel1D:
            return "texture_sample_level_1d";
        case LanguageFeature::kUnrestrictedPointerParameters:
            return "unrestricted_pointer_parameters";
    }
    return "<unknown>";
}

}  // namespace tint::wgsl
