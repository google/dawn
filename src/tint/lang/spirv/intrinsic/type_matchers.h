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

#ifndef SRC_TINT_LANG_SPIRV_INTRINSIC_TYPE_MATCHERS_H_
#define SRC_TINT_LANG_SPIRV_INTRINSIC_TYPE_MATCHERS_H_

#include "src/tint/lang/core/intrinsic/table.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/manager.h"
#include "src/tint/lang/core/type/struct.h"
#include "src/tint/lang/spirv/type/sampled_image.h"

namespace tint::spirv::intrinsic {

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

inline bool MatchSampledImage(core::intrinsic::MatchState&,
                              const core::type::Type* ty,
                              const core::type::Type*& T) {
    if (ty->Is<core::intrinsic::Any>()) {
        T = ty;
        return true;
    }
    if (auto* v = ty->As<spirv::type::SampledImage>()) {
        T = v->Image();
        return true;
    }
    return false;
}

inline const core::type::Type* BuildSampledImage(core::intrinsic::MatchState& state,
                                                 const core::type::Type*,
                                                 const core::type::Type* T) {
    return state.types.Get<type::SampledImage>(T);
}

}  // namespace tint::spirv::intrinsic

#endif  // SRC_TINT_LANG_SPIRV_INTRINSIC_TYPE_MATCHERS_H_
