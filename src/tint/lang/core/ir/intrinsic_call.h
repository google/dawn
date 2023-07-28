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

#ifndef SRC_TINT_LANG_CORE_IR_INTRINSIC_CALL_H_
#define SRC_TINT_LANG_CORE_IR_INTRINSIC_CALL_H_

#include "src/tint/lang/core/ir/call.h"
#include "src/tint/utils/rtti/castable.h"

namespace tint::ir {

/// A backend intrinsic call instruction in the IR.
class IntrinsicCall : public Castable<IntrinsicCall, Call> {
  public:
    /// The base offset in Operands() for the args
    static constexpr size_t kArgsOperandOffset = 0;

    /// The kind of instruction.
    enum class Kind {
        // SPIR-V backend intrinsics.
        kSpirvArrayLength,
        kSpirvAtomicAnd,
        kSpirvAtomicCompareExchange,
        kSpirvAtomicExchange,
        kSpirvAtomicIAdd,
        kSpirvAtomicISub,
        kSpirvAtomicLoad,
        kSpirvAtomicOr,
        kSpirvAtomicSMax,
        kSpirvAtomicSMin,
        kSpirvAtomicStore,
        kSpirvAtomicUMax,
        kSpirvAtomicUMin,
        kSpirvAtomicXor,
        kSpirvDot,
        kSpirvImageFetch,
        kSpirvImageGather,
        kSpirvImageDrefGather,
        kSpirvImageQuerySize,
        kSpirvImageQuerySizeLod,
        kSpirvImageSampleImplicitLod,
        kSpirvImageSampleExplicitLod,
        kSpirvImageSampleDrefImplicitLod,
        kSpirvImageSampleDrefExplicitLod,
        kSpirvImageWrite,
        kSpirvMatrixTimesMatrix,
        kSpirvMatrixTimesScalar,
        kSpirvMatrixTimesVector,
        kSpirvSampledImage,
        kSpirvSelect,
        kSpirvVectorTimesMatrix,
        kSpirvVectorTimesScalar,
    };

    /// Constructor
    /// @param result the result value
    /// @param kind the intrinsic kind
    /// @param args the intrinsic call arguments
    IntrinsicCall(InstructionResult* result, enum Kind kind, VectorRef<Value*> args = tint::Empty);
    ~IntrinsicCall() override;

    /// @returns the builtin function
    enum Kind Kind() { return kind_; }

    /// @returns the friendly name for the instruction
    std::string_view FriendlyName() override { return "intrinsic-call"; }

  private:
    enum Kind kind_;
};

/// @param kind the enum value
/// @returns the string for the given enum value
std::string_view ToString(enum IntrinsicCall::Kind kind);

/// Emits the name of the intrinsic type.
template <typename STREAM, typename = traits::EnableIfIsOStream<STREAM>>
auto& operator<<(STREAM& out, enum IntrinsicCall::Kind kind) {
    return out << ToString(kind);
}

}  // namespace tint::ir

#endif  // SRC_TINT_LANG_CORE_IR_INTRINSIC_CALL_H_
