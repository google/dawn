// Copyright 2021 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0(the "License");
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

#include "src/tint/sem/type_constructor.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::TypeConstructor);

namespace tint::sem {

TypeConstructor::TypeConstructor(const sem::Type* type, const ParameterList& parameters)
    : Base(type, parameters) {}

TypeConstructor::~TypeConstructor() = default;

}  // namespace tint::sem
