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
namespace {

utils::VectorRef<const Parameter*> SetOwner(utils::VectorRef<Parameter*> parameters,
                                            const tint::sem::CallTarget* owner) {
    for (auto* parameter : parameters) {
        parameter->SetOwner(owner);
    }
    return parameters;
}

}  // namespace

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
    return IsImageQueryBuiltin(i) ||                           //
           i == BuiltinType::kTextureGather ||                 //
           i == BuiltinType::kTextureGatherCompare ||          //
           i == BuiltinType::kTextureLoad ||                   //
           i == BuiltinType::kTextureSample ||                 //
           i == BuiltinType::kTextureSampleBaseClampToEdge ||  //
           i == BuiltinType::kTextureSampleBias ||             //
           i == BuiltinType::kTextureSampleCompare ||          //
           i == BuiltinType::kTextureSampleCompareLevel ||     //
           i == BuiltinType::kTextureSampleGrad ||             //
           i == BuiltinType::kTextureSampleLevel ||            //
           i == BuiltinType::kTextureStore;
}

bool IsImageQueryBuiltin(BuiltinType i) {
    return i == BuiltinType::kTextureDimensions || i == BuiltinType::kTextureNumLayers ||
           i == BuiltinType::kTextureNumLevels || i == BuiltinType::kTextureNumSamples;
}

bool IsDataPackingBuiltin(BuiltinType i) {
    return i == BuiltinType::kPack4X8Snorm || i == BuiltinType::kPack4X8Unorm ||
           i == BuiltinType::kPack2X16Snorm || i == BuiltinType::kPack2X16Unorm ||
           i == BuiltinType::kPack2X16Float;
}

bool IsDataUnpackingBuiltin(BuiltinType i) {
    return i == BuiltinType::kUnpack4X8Snorm || i == BuiltinType::kUnpack4X8Unorm ||
           i == BuiltinType::kUnpack2X16Snorm || i == BuiltinType::kUnpack2X16Unorm ||
           i == BuiltinType::kUnpack2X16Float;
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

bool IsDP4aBuiltin(BuiltinType i) {
    return i == sem::BuiltinType::kDot4I8Packed || i == sem::BuiltinType::kDot4U8Packed;
}

Builtin::Builtin(BuiltinType type,
                 const type::Type* return_type,
                 utils::VectorRef<Parameter*> parameters,
                 EvaluationStage eval_stage,
                 PipelineStageSet supported_stages,
                 bool is_deprecated,
                 bool must_use)
    : Base(return_type, SetOwner(std::move(parameters), this), eval_stage, must_use),
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
        case sem::BuiltinType::kAtomicAdd:
        case sem::BuiltinType::kAtomicAnd:
        case sem::BuiltinType::kAtomicCompareExchangeWeak:
        case sem::BuiltinType::kAtomicExchange:
        case sem::BuiltinType::kAtomicMax:
        case sem::BuiltinType::kAtomicMin:
        case sem::BuiltinType::kAtomicOr:
        case sem::BuiltinType::kAtomicStore:
        case sem::BuiltinType::kAtomicSub:
        case sem::BuiltinType::kAtomicXor:
        case sem::BuiltinType::kTextureStore:
        case sem::BuiltinType::kWorkgroupUniformLoad:
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
