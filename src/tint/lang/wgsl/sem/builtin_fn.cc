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

#include "src/tint/lang/wgsl/sem/builtin_fn.h"

#include <utility>
#include <vector>

#include "src/tint/utils/containers/transform.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::BuiltinFn);

namespace tint::sem {

const char* BuiltinFn::str() const {
    return wgsl::str(fn_);
}

BuiltinFn::BuiltinFn(wgsl::BuiltinFn type,
                     const core::type::Type* return_type,
                     VectorRef<Parameter*> parameters,
                     core::EvaluationStage eval_stage,
                     PipelineStageSet supported_stages,
                     bool is_deprecated,
                     bool must_use)
    : Base(return_type, std::move(parameters), eval_stage, must_use),
      fn_(type),
      supported_stages_(supported_stages),
      is_deprecated_(is_deprecated) {}

BuiltinFn::~BuiltinFn() = default;

bool BuiltinFn::IsCoarseDerivative() const {
    return wgsl::IsCoarseDerivative(fn_);
}

bool BuiltinFn::IsFineDerivative() const {
    return wgsl::IsFineDerivative(fn_);
}

bool BuiltinFn::IsDerivative() const {
    return wgsl::IsDerivative(fn_);
}

bool BuiltinFn::IsTexture() const {
    return wgsl::IsTexture(fn_);
}

bool BuiltinFn::IsImageQuery() const {
    return wgsl::IsImageQuery(fn_);
}

bool BuiltinFn::IsDataPacking() const {
    return wgsl::IsDataPacking(fn_);
}

bool BuiltinFn::IsDataUnpacking() const {
    return wgsl::IsDataUnpacking(fn_);
}

bool BuiltinFn::IsBarrier() const {
    return wgsl::IsBarrier(fn_);
}

bool BuiltinFn::IsAtomic() const {
    return wgsl::IsAtomic(fn_);
}

bool BuiltinFn::IsDP4a() const {
    return wgsl::IsDP4a(fn_);
}

bool BuiltinFn::IsSubgroup() const {
    return wgsl::IsSubgroup(fn_);
}

bool BuiltinFn::HasSideEffects() const {
    return wgsl::HasSideEffects(fn_);
}

wgsl::Extension BuiltinFn::RequiredExtension() const {
    if (IsDP4a()) {
        return wgsl::Extension::kChromiumExperimentalDp4A;
    }
    if (IsSubgroup()) {
        return wgsl::Extension::kChromiumExperimentalSubgroups;
    }
    if (fn_ == wgsl::BuiltinFn::kTextureBarrier) {
        return wgsl::Extension::kChromiumExperimentalReadWriteStorageTexture;
    }
    return wgsl::Extension::kUndefined;
}

}  // namespace tint::sem

//! @endcond
