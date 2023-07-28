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

StringStream& operator<<(StringStream& out, enum IntrinsicCall::Kind kind) {
    switch (kind) {
        case IntrinsicCall::Kind::kSpirvArrayLength:
            out << "spirv.array_length";
            break;
        case IntrinsicCall::Kind::kSpirvAtomicIAdd:
            out << "spirv.atomic_iadd";
            break;
        case IntrinsicCall::Kind::kSpirvAtomicISub:
            out << "spirv.atomic_isub";
            break;
        case IntrinsicCall::Kind::kSpirvAtomicAnd:
            out << "spirv.atomic_and";
            break;
        case IntrinsicCall::Kind::kSpirvAtomicCompareExchange:
            out << "spirv.atomic_compare_exchange";
            break;
        case IntrinsicCall::Kind::kSpirvAtomicExchange:
            out << "spirv.atomic_exchange";
            break;
        case IntrinsicCall::Kind::kSpirvAtomicLoad:
            out << "spirv.atomic_load";
            break;
        case IntrinsicCall::Kind::kSpirvAtomicOr:
            out << "spirv.atomic_or";
            break;
        case IntrinsicCall::Kind::kSpirvAtomicSMax:
            out << "spirv.atomic_smax";
            break;
        case IntrinsicCall::Kind::kSpirvAtomicSMin:
            out << "spirv.atomic_smin";
            break;
        case IntrinsicCall::Kind::kSpirvAtomicStore:
            out << "spirv.atomic_store";
            break;
        case IntrinsicCall::Kind::kSpirvAtomicUMax:
            out << "spirv.atomic_umax";
            break;
        case IntrinsicCall::Kind::kSpirvAtomicUMin:
            out << "spirv.atomic_umin";
            break;
        case IntrinsicCall::Kind::kSpirvAtomicXor:
            out << "spirv.atomic_xor";
            break;
        case IntrinsicCall::Kind::kSpirvDot:
            out << "spirv.dot";
            break;
        case IntrinsicCall::Kind::kSpirvImageFetch:
            out << "spirv.image_fetch";
            break;
        case IntrinsicCall::Kind::kSpirvImageGather:
            out << "spirv.image_gather";
            break;
        case IntrinsicCall::Kind::kSpirvImageDrefGather:
            out << "spirv.image_dref_gather";
            break;
        case IntrinsicCall::Kind::kSpirvImageQuerySize:
            out << "spirv.image_query_size";
            break;
        case IntrinsicCall::Kind::kSpirvImageQuerySizeLod:
            out << "spirv.image_query_size_lod";
            break;
        case IntrinsicCall::Kind::kSpirvImageSampleImplicitLod:
            out << "spirv.image_sample_implicit_lod";
            break;
        case IntrinsicCall::Kind::kSpirvImageSampleExplicitLod:
            out << "spirv.image_sample_explicit_lod";
            break;
        case IntrinsicCall::Kind::kSpirvImageSampleDrefImplicitLod:
            out << "spirv.image_sample_dref_implicit_lod";
            break;
        case IntrinsicCall::Kind::kSpirvImageSampleDrefExplicitLod:
            out << "spirv.image_sample_dref_implicit_lod";
            break;
        case IntrinsicCall::Kind::kSpirvImageWrite:
            out << "spirv.image_write";
            break;
        case IntrinsicCall::Kind::kSpirvMatrixTimesMatrix:
            out << "spirv.matrix_times_matrix";
            break;
        case IntrinsicCall::Kind::kSpirvMatrixTimesScalar:
            out << "spirv.matrix_times_scalar";
            break;
        case IntrinsicCall::Kind::kSpirvMatrixTimesVector:
            out << "spirv.matrix_times_vector";
            break;
        case IntrinsicCall::Kind::kSpirvSampledImage:
            out << "spirv.sampled_image";
            break;
        case IntrinsicCall::Kind::kSpirvSelect:
            out << "spirv.select";
            break;
        case IntrinsicCall::Kind::kSpirvVectorTimesScalar:
            out << "spirv.vector_times_scalar";
            break;
        case IntrinsicCall::Kind::kSpirvVectorTimesMatrix:
            out << "spirv.vector_times_matrix";
            break;
    }
    return out;
}

}  // namespace tint::ir
