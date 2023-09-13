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

#ifndef SRC_TINT_LANG_SPIRV_INTRINSIC_DATA_TYPE_MATCHERS_H_
#define SRC_TINT_LANG_SPIRV_INTRINSIC_DATA_TYPE_MATCHERS_H_

#include "src/tint/lang/core/intrinsic/table.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/struct.h"

namespace tint::spirv::intrinsic::data {

inline bool MatchStructWithRuntimeArray(core::intrinsic::MatchState&, const core::type::Type* ty) {
    if (auto* str = ty->As<core::type::Struct>()) {
        if (str->Members().IsEmpty()) {
            return false;
        }
        if (auto* ary = str->Members().Back()->Type()->As<core::type::Array>()) {
            if (ary->Count()->Is<core::type::RuntimeArrayCount>()) {
                return true;
            }
        }
    }
    return false;
}

inline const core::type::Type* BuildStructWithRuntimeArray(core::intrinsic::MatchState&,
                                                           const core::type::Type* ty) {
    return ty;
}

}  // namespace tint::spirv::intrinsic::data

#endif  // SRC_TINT_LANG_SPIRV_INTRINSIC_DATA_TYPE_MATCHERS_H_
