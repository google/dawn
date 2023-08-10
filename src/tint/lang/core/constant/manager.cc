// Copyright 2023 The Tint Authors.
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

#include "src/tint/lang/core/constant/manager.h"

#include "src/tint/lang/core/constant/composite.h"
#include "src/tint/lang/core/constant/scalar.h"
#include "src/tint/lang/core/constant/splat.h"
#include "src/tint/lang/core/type/abstract_float.h"
#include "src/tint/lang/core/type/abstract_int.h"
#include "src/tint/lang/core/type/bool.h"
#include "src/tint/lang/core/type/f16.h"
#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/i32.h"
#include "src/tint/lang/core/type/manager.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/utils/containers/predicates.h"

namespace tint::core::constant {

Manager::Manager() = default;

Manager::Manager(Manager&&) = default;

Manager& Manager::operator=(Manager&& rhs) = default;

Manager::~Manager() = default;

const constant::Value* Manager::Composite(const core::type::Type* type,
                                          VectorRef<const constant::Value*> elements) {
    if (elements.IsEmpty()) {
        return nullptr;
    }

    bool any_zero = false;
    bool all_zero = true;
    bool all_equal = true;
    auto* first = elements.Front();
    for (auto* el : elements) {
        if (!el) {
            return nullptr;
        }
        if (!any_zero && el->AnyZero()) {
            any_zero = true;
        }
        if (all_zero && !el->AllZero()) {
            all_zero = false;
        }
        if (all_equal && el != first) {
            all_equal = false;
        }
    }
    if (all_equal) {
        return Splat(type, elements.Front(), elements.Length());
    }

    return Get<constant::Composite>(type, std::move(elements), all_zero, any_zero);
}

const constant::Splat* Manager::Splat(const core::type::Type* type,
                                      const constant::Value* element,
                                      size_t n) {
    return Get<constant::Splat>(type, element, n);
}

const Scalar<i32>* Manager::Get(i32 value) {
    return Get<Scalar<i32>>(types.i32(), value);
}

const Scalar<u32>* Manager::Get(u32 value) {
    return Get<Scalar<u32>>(types.u32(), value);
}

const Scalar<f32>* Manager::Get(f32 value) {
    return Get<Scalar<f32>>(types.f32(), value);
}

const Scalar<f16>* Manager::Get(f16 value) {
    return Get<Scalar<f16>>(types.f16(), value);
}

const Scalar<bool>* Manager::Get(bool value) {
    return Get<Scalar<bool>>(types.bool_(), value);
}

const Scalar<AFloat>* Manager::Get(AFloat value) {
    return Get<Scalar<AFloat>>(types.AFloat(), value);
}

const Scalar<AInt>* Manager::Get(AInt value) {
    return Get<Scalar<AInt>>(types.AInt(), value);
}

}  // namespace tint::core::constant
