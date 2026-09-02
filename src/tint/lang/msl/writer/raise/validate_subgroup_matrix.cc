// Copyright 2025 The Dawn & Tint Authors
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

#include "src/tint/lang/msl/writer/raise/validate_subgroup_matrix.h"

#include <cstdint>
#include <utility>

#include "src/tint/lang/core/ir/analysis/subgroup_matrix.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/validator/validate.h"
#include "src/tint/lang/core/type/manager.h"

namespace tint::msl::writer::raise {
namespace {

using namespace tint::core::fluent_types;  // NOLINT

struct State {
    /// The IR module.
    core::ir::Module& ir;

    diag::List diagnostics_{};

    /// Process the module.
    diag::Result<SuccessType> Process(bool enable_tensors) {
        auto info = core::ir::analysis::GatherSubgroupMatrixInfo(ir);

        for (auto& i : info.configs) {
            if (enable_tensors) {
                // All dimensions must be 16 or 32.
                if (!IsValidCooperativeTensorDim(i.M) || !IsValidCooperativeTensorDim(i.N) ||
                    !IsValidCooperativeTensorDim(i.K)) {
                    diagnostics_.AddError(Source{})
                        << "Pipeline uses subgroup matrix dimensions that are not supported by the "
                           "device ("
                        << (i.M != 0 ? i.M : i.K) << "x" << ((i.N != 0 ? i.N : i.K)) << ")";
                    break;
                }
            } else {
                if ((i.direction == SubgroupMatrixDirection::kLeft && (i.M != 8 || i.K != 8)) ||
                    (i.direction == SubgroupMatrixDirection::kRight && (i.K != 8 || i.N != 8)) ||
                    (i.direction == SubgroupMatrixDirection::kResult && (i.M != 8 || i.N != 8))) {
                    diagnostics_.AddError(Source{})
                        << "Pipeline uses subgroup matrix dimensions that are not supported by the "
                           "device ("
                        << (i.M != 0 ? i.M : i.K) << "x" << ((i.N != 0 ? i.N : i.K)) << ")";
                    break;
                }

                if (i.type != SubgroupMatrixType::kF32 && i.type != SubgroupMatrixType::kF16) {
                    diagnostics_.AddError(Source{}) << "subgroup_matrix requires a type of `f32` "
                                                       "or `f16` for the selected device";
                    break;
                }
            }
        }

        for (auto& i : info.multiplies) {
            if (enable_tensors) {
                // At least one dimension must be 32.
                if (i.M != 32 || i.N != 32 || i.K != 32) {
                    diagnostics_.AddError(Source{}) << "Pipeline uses subgroup matrix multiply "
                                                       "shape that is not supported by the device ("
                                                    << i.M << "x" << i.N << "x" << i.K << ")";
                    break;
                }
            }
        }

        if (!diagnostics_.IsEmpty()) {
            return diag::Failure{diagnostics_};
        }
        return Success;
    }

    bool IsValidCooperativeTensorDim(uint32_t d) {
        // cooperative_tensor dimensions must all be 16 or 32.
        // allow zero as well which signals that the dimension is unused for the given usage kind.
        return d == 0 || d == 16 || d == 32;
    }
};

}  // namespace

Result<SuccessType> ValidateSubgroupMatrix(core::ir::Module& ir, bool enable_tensors) {
    AssertValid(ir, "before msl.ValidateSubgroupMatrix");

    auto res = State{ir}.Process(enable_tensors);
    if (res != Success) {
        return Failure{res.Failure().reason.Str()};
    }

    return Success;
}

}  // namespace tint::msl::writer::raise
