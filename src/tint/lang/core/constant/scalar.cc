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

#include "src/tint/lang/core/constant/scalar.h"

TINT_INSTANTIATE_TYPEINFO(tint::core::constant::ScalarBase);
TINT_INSTANTIATE_TYPEINFO(tint::core::constant::Scalar<tint::core::AInt>);
TINT_INSTANTIATE_TYPEINFO(tint::core::constant::Scalar<tint::core::AFloat>);
TINT_INSTANTIATE_TYPEINFO(tint::core::constant::Scalar<tint::core::i32>);
TINT_INSTANTIATE_TYPEINFO(tint::core::constant::Scalar<tint::core::u32>);
TINT_INSTANTIATE_TYPEINFO(tint::core::constant::Scalar<tint::core::f16>);
TINT_INSTANTIATE_TYPEINFO(tint::core::constant::Scalar<tint::core::f32>);
TINT_INSTANTIATE_TYPEINFO(tint::core::constant::Scalar<bool>);

namespace tint::core::constant {

ScalarBase::~ScalarBase() = default;

}
