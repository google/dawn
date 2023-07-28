// Copyright 2021 The Tint Authors.
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

#include "src/tint/lang/wgsl/sem/call_target.h"

#include <utility>

#include "src/tint/utils/math/hash.h"
#include "src/tint/utils/text/symbol_table.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::CallTarget);

namespace tint::sem {

CallTarget::CallTarget(EvaluationStage stage, bool must_use) : stage_(stage), must_use_(must_use) {}

CallTarget::CallTarget(const type::Type* return_type,
                       VectorRef<Parameter*> parameters,
                       EvaluationStage stage,
                       bool must_use)
    : stage_(stage), must_use_(must_use) {
    SetReturnType(return_type);
    for (auto* param : parameters) {
        AddParameter(param);
    }
    TINT_ASSERT(return_type);
}

CallTarget::CallTarget(const CallTarget&) = default;
CallTarget::~CallTarget() = default;

CallTargetSignature::CallTargetSignature() = default;

CallTargetSignature::CallTargetSignature(const type::Type* ret_ty,
                                         VectorRef<const sem::Parameter*> params)
    : return_type(ret_ty), parameters(std::move(params)) {}
CallTargetSignature::CallTargetSignature(const CallTargetSignature&) = default;
CallTargetSignature::~CallTargetSignature() = default;

int CallTargetSignature::IndexOf(ParameterUsage usage) const {
    for (size_t i = 0; i < parameters.Length(); i++) {
        if (parameters[i]->Usage() == usage) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

bool CallTargetSignature::operator==(const CallTargetSignature& other) const {
    if (return_type != other.return_type || parameters.Length() != other.parameters.Length()) {
        return false;
    }
    for (size_t i = 0; i < parameters.Length(); i++) {
        auto* a = parameters[i];
        auto* b = other.parameters[i];
        if (a->Type() != b->Type() || a->Usage() != b->Usage()) {
            return false;
        }
    }
    return true;
}

}  // namespace tint::sem

namespace std {

std::size_t hash<tint::sem::CallTargetSignature>::operator()(
    const tint::sem::CallTargetSignature& sig) const {
    size_t hash = tint::Hash(sig.parameters.Length());
    for (auto* p : sig.parameters) {
        hash = HashCombine(hash, p->Type(), p->Usage());
    }
    return Hash(hash, sig.return_type);
}

}  // namespace std
