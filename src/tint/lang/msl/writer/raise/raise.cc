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

#include "src/tint/lang/msl/writer/raise/raise.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/binary_polyfill.h"
#include "src/tint/lang/core/ir/transform/binding_remapper.h"
#include "src/tint/lang/core/ir/transform/builtin_polyfill.h"
#include "src/tint/lang/core/ir/transform/conversion_polyfill.h"
#include "src/tint/lang/core/ir/transform/demote_to_helper.h"
#include "src/tint/lang/core/ir/transform/multiplanar_external_texture.h"
#include "src/tint/lang/core/ir/transform/preserve_padding.h"
#include "src/tint/lang/core/ir/transform/robustness.h"
#include "src/tint/lang/core/ir/transform/value_to_let.h"
#include "src/tint/lang/core/ir/transform/vectorize_scalar_matrix_constructors.h"
#include "src/tint/lang/core/ir/transform/zero_init_workgroup_memory.h"
#include "src/tint/lang/msl/writer/common/option_helpers.h"
#include "src/tint/lang/msl/writer/raise/builtin_polyfill.h"
#include "src/tint/lang/msl/writer/raise/module_scope_vars.h"

namespace tint::msl::writer {

Result<SuccessType> Raise(core::ir::Module& module, const Options& options) {
#define RUN_TRANSFORM(name, ...)                   \
    do {                                           \
        auto result = name(module, ##__VA_ARGS__); \
        if (result != Success) {                   \
            return result;                         \
        }                                          \
    } while (false)

    ExternalTextureOptions external_texture_options{};
    RemapperData remapper_data{};
    ArrayLengthFromUniformOptions array_length_from_uniform_options{};
    PopulateBindingRelatedOptions(options, remapper_data, external_texture_options,
                                  array_length_from_uniform_options);
    RUN_TRANSFORM(core::ir::transform::BindingRemapper, remapper_data);

    {
        core::ir::transform::BinaryPolyfillConfig binary_polyfills{};
        binary_polyfills.int_div_mod = !options.disable_polyfill_integer_div_mod;
        binary_polyfills.bitshift_modulo = true;  // crbug.com/tint/1543
        RUN_TRANSFORM(core::ir::transform::BinaryPolyfill, binary_polyfills);
    }

    {
        core::ir::transform::BuiltinPolyfillConfig core_polyfills{};
        core_polyfills.clamp_int = true;
        core_polyfills.extract_bits = core::ir::transform::BuiltinPolyfillLevel::kClampOrRangeCheck;
        core_polyfills.first_leading_bit = true;
        core_polyfills.first_trailing_bit = true;
        core_polyfills.insert_bits = core::ir::transform::BuiltinPolyfillLevel::kClampOrRangeCheck;
        core_polyfills.texture_sample_base_clamp_to_edge_2d_f32 = true;
        RUN_TRANSFORM(core::ir::transform::BuiltinPolyfill, core_polyfills);
    }
    // polyfills.sign_int = true;

    {
        core::ir::transform::ConversionPolyfillConfig conversion_polyfills;
        conversion_polyfills.ftoi = true;
        RUN_TRANSFORM(core::ir::transform::ConversionPolyfill, conversion_polyfills);
    }

    if (!options.disable_robustness) {
        core::ir::transform::RobustnessConfig config{};
        RUN_TRANSFORM(core::ir::transform::Robustness, config);
    }

    RUN_TRANSFORM(core::ir::transform::MultiplanarExternalTexture, external_texture_options);

    if (!options.disable_workgroup_init) {
        RUN_TRANSFORM(core::ir::transform::ZeroInitWorkgroupMemory);
    }

    RUN_TRANSFORM(core::ir::transform::PreservePadding);
    RUN_TRANSFORM(core::ir::transform::VectorizeScalarMatrixConstructors);

    // DemoteToHelper must come before any transform that introduces non-core instructions.
    RUN_TRANSFORM(core::ir::transform::DemoteToHelper);

    RUN_TRANSFORM(raise::ModuleScopeVars);
    RUN_TRANSFORM(core::ir::transform::ValueToLet);
    RUN_TRANSFORM(raise::BuiltinPolyfill);

    return Success;
}

}  // namespace tint::msl::writer
