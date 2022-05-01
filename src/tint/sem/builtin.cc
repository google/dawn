// Copyright 2020 The Tint Authors.
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

// Doxygen seems to trip over this file for some unknown reason. Disable.
//! @cond Doxygen_Suppress

#include "src/tint/sem/builtin.h"

#include <vector>

#include "src/tint/utils/to_const_ptr_vec.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::Builtin);

namespace tint::sem {

const char* Builtin::str() const {
    return sem::str(type_);
}

bool IsCoarseDerivativeBuiltin(BuiltinType i) {
    return i == BuiltinType::kDpdxCoarse || i == BuiltinType::kDpdyCoarse ||
           i == BuiltinType::kFwidthCoarse;
}

bool IsFineDerivativeBuiltin(BuiltinType i) {
    return i == BuiltinType::kDpdxFine || i == BuiltinType::kDpdyFine ||
           i == BuiltinType::kFwidthFine;
}

bool IsDerivativeBuiltin(BuiltinType i) {
    return i == BuiltinType::kDpdx || i == BuiltinType::kDpdy || i == BuiltinType::kFwidth ||
           IsCoarseDerivativeBuiltin(i) || IsFineDerivativeBuiltin(i);
}

bool IsTextureBuiltin(BuiltinType i) {
    return IsImageQueryBuiltin(i) || i == BuiltinType::kTextureLoad ||
           i == BuiltinType::kTextureGather || i == BuiltinType::kTextureGatherCompare ||
           i == BuiltinType::kTextureSample || i == BuiltinType::kTextureSampleLevel ||
           i == BuiltinType::kTextureSampleBias || i == BuiltinType::kTextureSampleCompare ||
           i == BuiltinType::kTextureSampleCompareLevel || i == BuiltinType::kTextureSampleGrad ||
           i == BuiltinType::kTextureStore;
}

bool IsImageQueryBuiltin(BuiltinType i) {
    return i == BuiltinType::kTextureDimensions || i == BuiltinType::kTextureNumLayers ||
           i == BuiltinType::kTextureNumLevels || i == BuiltinType::kTextureNumSamples;
}

bool IsDataPackingBuiltin(BuiltinType i) {
    return i == BuiltinType::kPack4x8snorm || i == BuiltinType::kPack4x8unorm ||
           i == BuiltinType::kPack2x16snorm || i == BuiltinType::kPack2x16unorm ||
           i == BuiltinType::kPack2x16float;
}

bool IsDataUnpackingBuiltin(BuiltinType i) {
    return i == BuiltinType::kUnpack4x8snorm || i == BuiltinType::kUnpack4x8unorm ||
           i == BuiltinType::kUnpack2x16snorm || i == BuiltinType::kUnpack2x16unorm ||
           i == BuiltinType::kUnpack2x16float;
}

bool IsBarrierBuiltin(BuiltinType i) {
    return i == BuiltinType::kWorkgroupBarrier || i == BuiltinType::kStorageBarrier;
}

bool IsAtomicBuiltin(BuiltinType i) {
    return i == sem::BuiltinType::kAtomicLoad || i == sem::BuiltinType::kAtomicStore ||
           i == sem::BuiltinType::kAtomicAdd || i == sem::BuiltinType::kAtomicSub ||
           i == sem::BuiltinType::kAtomicMax || i == sem::BuiltinType::kAtomicMin ||
           i == sem::BuiltinType::kAtomicAnd || i == sem::BuiltinType::kAtomicOr ||
           i == sem::BuiltinType::kAtomicXor || i == sem::BuiltinType::kAtomicExchange ||
           i == sem::BuiltinType::kAtomicCompareExchangeWeak;
}

Builtin::Builtin(BuiltinType type,
                 const sem::Type* return_type,
                 std::vector<Parameter*> parameters,
                 PipelineStageSet supported_stages,
                 bool is_deprecated)
    : Base(return_type, utils::ToConstPtrVec(parameters)),
      type_(type),
      supported_stages_(supported_stages),
      is_deprecated_(is_deprecated) {
    for (auto* parameter : parameters) {
        parameter->SetOwner(this);
    }
}

Builtin::~Builtin() = default;

bool Builtin::IsCoarseDerivative() const {
    return IsCoarseDerivativeBuiltin(type_);
}

bool Builtin::IsFineDerivative() const {
    return IsFineDerivativeBuiltin(type_);
}

bool Builtin::IsDerivative() const {
    return IsDerivativeBuiltin(type_);
}

bool Builtin::IsTexture() const {
    return IsTextureBuiltin(type_);
}

bool Builtin::IsImageQuery() const {
    return IsImageQueryBuiltin(type_);
}

bool Builtin::IsDataPacking() const {
    return IsDataPackingBuiltin(type_);
}

bool Builtin::IsDataUnpacking() const {
    return IsDataUnpackingBuiltin(type_);
}

bool Builtin::IsBarrier() const {
    return IsBarrierBuiltin(type_);
}

bool Builtin::IsAtomic() const {
    return IsAtomicBuiltin(type_);
}

bool Builtin::HasSideEffects() const {
    if (IsAtomic() && type_ != sem::BuiltinType::kAtomicLoad) {
        return true;
    }
    if (type_ == sem::BuiltinType::kTextureStore) {
        return true;
    }
    return false;
}

}  // namespace tint::sem

//! @endcond
