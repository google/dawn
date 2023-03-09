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

#include <utility>
#include <vector>

#include "src/tint/utils/transform.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::Builtin);

namespace tint::sem {

const char* Builtin::str() const {
    return builtin::str(type_);
}

bool IsCoarseDerivativeBuiltin(builtin::Function i) {
    return i == builtin::Function::kDpdxCoarse || i == builtin::Function::kDpdyCoarse ||
           i == builtin::Function::kFwidthCoarse;
}

bool IsFineDerivativeBuiltin(builtin::Function i) {
    return i == builtin::Function::kDpdxFine || i == builtin::Function::kDpdyFine ||
           i == builtin::Function::kFwidthFine;
}

bool IsDerivativeBuiltin(builtin::Function i) {
    return i == builtin::Function::kDpdx || i == builtin::Function::kDpdy ||
           i == builtin::Function::kFwidth || IsCoarseDerivativeBuiltin(i) ||
           IsFineDerivativeBuiltin(i);
}

bool IsTextureBuiltin(builtin::Function i) {
    return IsImageQueryBuiltin(i) ||                                 //
           i == builtin::Function::kTextureGather ||                 //
           i == builtin::Function::kTextureGatherCompare ||          //
           i == builtin::Function::kTextureLoad ||                   //
           i == builtin::Function::kTextureSample ||                 //
           i == builtin::Function::kTextureSampleBaseClampToEdge ||  //
           i == builtin::Function::kTextureSampleBias ||             //
           i == builtin::Function::kTextureSampleCompare ||          //
           i == builtin::Function::kTextureSampleCompareLevel ||     //
           i == builtin::Function::kTextureSampleGrad ||             //
           i == builtin::Function::kTextureSampleLevel ||            //
           i == builtin::Function::kTextureStore;
}

bool IsImageQueryBuiltin(builtin::Function i) {
    return i == builtin::Function::kTextureDimensions ||
           i == builtin::Function::kTextureNumLayers || i == builtin::Function::kTextureNumLevels ||
           i == builtin::Function::kTextureNumSamples;
}

bool IsDataPackingBuiltin(builtin::Function i) {
    return i == builtin::Function::kPack4X8Snorm || i == builtin::Function::kPack4X8Unorm ||
           i == builtin::Function::kPack2X16Snorm || i == builtin::Function::kPack2X16Unorm ||
           i == builtin::Function::kPack2X16Float;
}

bool IsDataUnpackingBuiltin(builtin::Function i) {
    return i == builtin::Function::kUnpack4X8Snorm || i == builtin::Function::kUnpack4X8Unorm ||
           i == builtin::Function::kUnpack2X16Snorm || i == builtin::Function::kUnpack2X16Unorm ||
           i == builtin::Function::kUnpack2X16Float;
}

bool IsBarrierBuiltin(builtin::Function i) {
    return i == builtin::Function::kWorkgroupBarrier || i == builtin::Function::kStorageBarrier;
}

bool IsAtomicBuiltin(builtin::Function i) {
    return i == builtin::Function::kAtomicLoad || i == builtin::Function::kAtomicStore ||
           i == builtin::Function::kAtomicAdd || i == builtin::Function::kAtomicSub ||
           i == builtin::Function::kAtomicMax || i == builtin::Function::kAtomicMin ||
           i == builtin::Function::kAtomicAnd || i == builtin::Function::kAtomicOr ||
           i == builtin::Function::kAtomicXor || i == builtin::Function::kAtomicExchange ||
           i == builtin::Function::kAtomicCompareExchangeWeak;
}

bool IsDP4aBuiltin(builtin::Function i) {
    return i == builtin::Function::kDot4I8Packed || i == builtin::Function::kDot4U8Packed;
}

Builtin::Builtin(builtin::Function type,
                 const type::Type* return_type,
                 utils::VectorRef<Parameter*> parameters,
                 EvaluationStage eval_stage,
                 PipelineStageSet supported_stages,
                 bool is_deprecated,
                 bool must_use)
    : Base(return_type, std::move(parameters), eval_stage, must_use),
      type_(type),
      supported_stages_(supported_stages),
      is_deprecated_(is_deprecated) {}

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

bool Builtin::IsDP4a() const {
    return IsDP4aBuiltin(type_);
}

bool Builtin::HasSideEffects() const {
    switch (type_) {
        case builtin::Function::kAtomicAdd:
        case builtin::Function::kAtomicAnd:
        case builtin::Function::kAtomicCompareExchangeWeak:
        case builtin::Function::kAtomicExchange:
        case builtin::Function::kAtomicMax:
        case builtin::Function::kAtomicMin:
        case builtin::Function::kAtomicOr:
        case builtin::Function::kAtomicStore:
        case builtin::Function::kAtomicSub:
        case builtin::Function::kAtomicXor:
        case builtin::Function::kTextureStore:
        case builtin::Function::kWorkgroupUniformLoad:
            return true;
        default:
            break;
    }
    return false;
}

builtin::Extension Builtin::RequiredExtension() const {
    if (IsDP4a()) {
        return builtin::Extension::kChromiumExperimentalDp4A;
    }
    return builtin::Extension::kUndefined;
}

}  // namespace tint::sem

//! @endcond
