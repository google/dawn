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

#include "src/sem/intrinsic.h"

#include <vector>

#include "src/utils/to_const_ptr_vec.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::Intrinsic);

namespace tint {
namespace sem {

const char* Intrinsic::str() const {
  return sem::str(type_);
}

bool IsCoarseDerivativeIntrinsic(IntrinsicType i) {
  return i == IntrinsicType::kDpdxCoarse || i == IntrinsicType::kDpdyCoarse ||
         i == IntrinsicType::kFwidthCoarse;
}

bool IsFineDerivativeIntrinsic(IntrinsicType i) {
  return i == IntrinsicType::kDpdxFine || i == IntrinsicType::kDpdyFine ||
         i == IntrinsicType::kFwidthFine;
}

bool IsDerivativeIntrinsic(IntrinsicType i) {
  return i == IntrinsicType::kDpdx || i == IntrinsicType::kDpdy ||
         i == IntrinsicType::kFwidth || IsCoarseDerivativeIntrinsic(i) ||
         IsFineDerivativeIntrinsic(i);
}

bool IsFloatClassificationIntrinsic(IntrinsicType i) {
  return i == IntrinsicType::kIsFinite || i == IntrinsicType::kIsInf ||
         i == IntrinsicType::kIsNan || i == IntrinsicType::kIsNormal;
}

bool IsTextureIntrinsic(IntrinsicType i) {
  return IsImageQueryIntrinsic(i) || i == IntrinsicType::kTextureLoad ||
         i == IntrinsicType::kTextureGather ||
         i == IntrinsicType::kTextureGatherCompare ||
         i == IntrinsicType::kTextureSample ||
         i == IntrinsicType::kTextureSampleLevel ||
         i == IntrinsicType::kTextureSampleBias ||
         i == IntrinsicType::kTextureSampleCompare ||
         i == IntrinsicType::kTextureSampleCompareLevel ||
         i == IntrinsicType::kTextureSampleGrad ||
         i == IntrinsicType::kTextureStore;
}

bool IsImageQueryIntrinsic(IntrinsicType i) {
  return i == IntrinsicType::kTextureDimensions ||
         i == IntrinsicType::kTextureNumLayers ||
         i == IntrinsicType::kTextureNumLevels ||
         i == IntrinsicType::kTextureNumSamples;
}

bool IsDataPackingIntrinsic(IntrinsicType i) {
  return i == IntrinsicType::kPack4x8snorm ||
         i == IntrinsicType::kPack4x8unorm ||
         i == IntrinsicType::kPack2x16snorm ||
         i == IntrinsicType::kPack2x16unorm ||
         i == IntrinsicType::kPack2x16float;
}

bool IsDataUnpackingIntrinsic(IntrinsicType i) {
  return i == IntrinsicType::kUnpack4x8snorm ||
         i == IntrinsicType::kUnpack4x8unorm ||
         i == IntrinsicType::kUnpack2x16snorm ||
         i == IntrinsicType::kUnpack2x16unorm ||
         i == IntrinsicType::kUnpack2x16float;
}

bool IsBarrierIntrinsic(IntrinsicType i) {
  return i == IntrinsicType::kWorkgroupBarrier ||
         i == IntrinsicType::kStorageBarrier;
}

bool IsAtomicIntrinsic(IntrinsicType i) {
  return i == sem::IntrinsicType::kAtomicLoad ||
         i == sem::IntrinsicType::kAtomicStore ||
         i == sem::IntrinsicType::kAtomicAdd ||
         i == sem::IntrinsicType::kAtomicSub ||
         i == sem::IntrinsicType::kAtomicMax ||
         i == sem::IntrinsicType::kAtomicMin ||
         i == sem::IntrinsicType::kAtomicAnd ||
         i == sem::IntrinsicType::kAtomicOr ||
         i == sem::IntrinsicType::kAtomicXor ||
         i == sem::IntrinsicType::kAtomicExchange ||
         i == sem::IntrinsicType::kAtomicCompareExchangeWeak;
}

Intrinsic::Intrinsic(IntrinsicType type,
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

Intrinsic::~Intrinsic() = default;

bool Intrinsic::IsCoarseDerivative() const {
  return IsCoarseDerivativeIntrinsic(type_);
}

bool Intrinsic::IsFineDerivative() const {
  return IsFineDerivativeIntrinsic(type_);
}

bool Intrinsic::IsDerivative() const {
  return IsDerivativeIntrinsic(type_);
}

bool Intrinsic::IsFloatClassification() const {
  return IsFloatClassificationIntrinsic(type_);
}

bool Intrinsic::IsTexture() const {
  return IsTextureIntrinsic(type_);
}

bool Intrinsic::IsImageQuery() const {
  return IsImageQueryIntrinsic(type_);
}

bool Intrinsic::IsDataPacking() const {
  return IsDataPackingIntrinsic(type_);
}

bool Intrinsic::IsDataUnpacking() const {
  return IsDataUnpackingIntrinsic(type_);
}

bool Intrinsic::IsBarrier() const {
  return IsBarrierIntrinsic(type_);
}

bool Intrinsic::IsAtomic() const {
  return IsAtomicIntrinsic(type_);
}

}  // namespace sem
}  // namespace tint
