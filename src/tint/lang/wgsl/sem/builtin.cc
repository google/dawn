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

#include "src/tint/lang/wgsl/sem/builtin.h"

#include <utility>
#include <vector>

#include "src/tint/utils/containers/transform.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::Builtin);

namespace tint::sem {

const char* Builtin::str() const {
    return core::str(type_);
}

Builtin::Builtin(core::Function type,
                 const type::Type* return_type,
                 VectorRef<Parameter*> parameters,
                 core::EvaluationStage eval_stage,
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

bool Builtin::IsSubgroup() const {
    return IsSubgroupBuiltin(type_);
}

bool Builtin::HasSideEffects() const {
    return core::HasSideEffects(type_);
}

core::Extension Builtin::RequiredExtension() const {
    if (IsDP4a()) {
        return core::Extension::kChromiumExperimentalDp4A;
    }
    if (IsSubgroup()) {
        return core::Extension::kChromiumExperimentalSubgroups;
    }
    return core::Extension::kUndefined;
}

}  // namespace tint::sem

//! @endcond
