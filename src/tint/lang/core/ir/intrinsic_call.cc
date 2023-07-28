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

#include "src/tint/lang/core/ir/intrinsic_call.h"

#include <utility>

TINT_INSTANTIATE_TYPEINFO(tint::ir::IntrinsicCall);

namespace tint::ir {

IntrinsicCall::IntrinsicCall(InstructionResult* result, enum Kind kind, VectorRef<Value*> arguments)
    : kind_(kind) {
    AddOperands(IntrinsicCall::kArgsOperandOffset, std::move(arguments));
    AddResult(result);
}

IntrinsicCall::~IntrinsicCall() = default;

std::string_view ToString(enum IntrinsicCall::Kind kind) {
    switch (kind) {
        case IntrinsicCall::Kind::kSpirvArrayLength:
            return "spirv.array_length";
        case IntrinsicCall::Kind::kSpirvAtomicIAdd:
            return "spirv.atomic_iadd";
        case IntrinsicCall::Kind::kSpirvAtomicISub:
            return "spirv.atomic_isub";
        case IntrinsicCall::Kind::kSpirvAtomicAnd:
            return "spirv.atomic_and";
        case IntrinsicCall::Kind::kSpirvAtomicCompareExchange:
            return "spirv.atomic_compare_exchange";
        case IntrinsicCall::Kind::kSpirvAtomicExchange:
            return "spirv.atomic_exchange";
        case IntrinsicCall::Kind::kSpirvAtomicLoad:
            return "spirv.atomic_load";
        case IntrinsicCall::Kind::kSpirvAtomicOr:
            return "spirv.atomic_or";
        case IntrinsicCall::Kind::kSpirvAtomicSMax:
            return "spirv.atomic_smax";
        case IntrinsicCall::Kind::kSpirvAtomicSMin:
            return "spirv.atomic_smin";
        case IntrinsicCall::Kind::kSpirvAtomicStore:
            return "spirv.atomic_store";
        case IntrinsicCall::Kind::kSpirvAtomicUMax:
            return "spirv.atomic_umax";
        case IntrinsicCall::Kind::kSpirvAtomicUMin:
            return "spirv.atomic_umin";
        case IntrinsicCall::Kind::kSpirvAtomicXor:
            return "spirv.atomic_xor";
        case IntrinsicCall::Kind::kSpirvDot:
            return "spirv.dot";
        case IntrinsicCall::Kind::kSpirvImageFetch:
            return "spirv.image_fetch";
        case IntrinsicCall::Kind::kSpirvImageGather:
            return "spirv.image_gather";
        case IntrinsicCall::Kind::kSpirvImageDrefGather:
            return "spirv.image_dref_gather";
        case IntrinsicCall::Kind::kSpirvImageQuerySize:
            return "spirv.image_query_size";
        case IntrinsicCall::Kind::kSpirvImageQuerySizeLod:
            return "spirv.image_query_size_lod";
        case IntrinsicCall::Kind::kSpirvImageSampleImplicitLod:
            return "spirv.image_sample_implicit_lod";
        case IntrinsicCall::Kind::kSpirvImageSampleExplicitLod:
            return "spirv.image_sample_explicit_lod";
        case IntrinsicCall::Kind::kSpirvImageSampleDrefImplicitLod:
            return "spirv.image_sample_dref_implicit_lod";
        case IntrinsicCall::Kind::kSpirvImageSampleDrefExplicitLod:
            return "spirv.image_sample_dref_implicit_lod";
        case IntrinsicCall::Kind::kSpirvImageWrite:
            return "spirv.image_write";
        case IntrinsicCall::Kind::kSpirvMatrixTimesMatrix:
            return "spirv.matrix_times_matrix";
        case IntrinsicCall::Kind::kSpirvMatrixTimesScalar:
            return "spirv.matrix_times_scalar";
        case IntrinsicCall::Kind::kSpirvMatrixTimesVector:
            return "spirv.matrix_times_vector";
        case IntrinsicCall::Kind::kSpirvSampledImage:
            return "spirv.sampled_image";
        case IntrinsicCall::Kind::kSpirvSelect:
            return "spirv.select";
        case IntrinsicCall::Kind::kSpirvVectorTimesScalar:
            return "spirv.vector_times_scalar";
        case IntrinsicCall::Kind::kSpirvVectorTimesMatrix:
            return "spirv.vector_times_matrix";
    }
    return "<unknown>";
}

}  // namespace tint::ir
