// Copyright 2022 The Tint Authors.
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

#include "src/tint/ir/constant.h"

#include <string>

#include "src/tint/constant/composite.h"
#include "src/tint/constant/scalar.h"
#include "src/tint/constant/splat.h"

TINT_INSTANTIATE_TYPEINFO(tint::ir::Constant);

namespace tint::ir {

Constant::Constant(const constant::Value* val) : value(val) {}

Constant::~Constant() = default;

std::ostream& Constant::ToString(std::ostream& out, const SymbolTable& st) const {
    std::function<void(const constant::Value*)> emit = [&](const constant::Value* c) {
        Switch(
            c,
            [&](const constant::Scalar<AFloat>* scalar) { out << scalar->ValueAs<AFloat>().value; },
            [&](const constant::Scalar<AInt>* scalar) { out << scalar->ValueAs<AInt>().value; },
            [&](const constant::Scalar<i32>* scalar) { out << scalar->ValueAs<i32>().value; },
            [&](const constant::Scalar<u32>* scalar) { out << scalar->ValueAs<u32>().value; },
            [&](const constant::Scalar<f32>* scalar) { out << scalar->ValueAs<f32>().value; },
            [&](const constant::Scalar<f16>* scalar) { out << scalar->ValueAs<f16>().value; },
            [&](const constant::Scalar<bool>* scalar) {
                out << (scalar->ValueAs<bool>() ? "true" : "false");
            },
            [&](const constant::Splat* splat) {
                out << splat->Type()->FriendlyName(st) << "(";
                emit(splat->Index(0));
                out << ")";
            },
            [&](const constant::Composite* composite) {
                out << composite->Type()->FriendlyName(st) << "(";
                for (const auto* elem : composite->elements) {
                    if (elem != composite->elements[0]) {
                        out << ", ";
                    }
                    emit(elem);
                }
                out << ")";
            });
    };
    emit(value);
    return out;
}

}  // namespace tint::ir
