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

#include "src/tint/type/manager.h"

#include "src/tint/type/abstract_float.h"
#include "src/tint/type/abstract_int.h"
#include "src/tint/type/array.h"
#include "src/tint/type/bool.h"
#include "src/tint/type/f16.h"
#include "src/tint/type/f32.h"
#include "src/tint/type/i32.h"
#include "src/tint/type/matrix.h"
#include "src/tint/type/pointer.h"
#include "src/tint/type/type.h"
#include "src/tint/type/u32.h"
#include "src/tint/type/vector.h"
#include "src/tint/type/void.h"

namespace tint::type {

Manager::Manager() = default;

Manager::Manager(Manager&&) = default;

Manager& Manager::operator=(Manager&& rhs) = default;

Manager::~Manager() = default;

const type::Void* Manager::void_() {
    return Get<type::Void>();
}

const type::Bool* Manager::bool_() {
    return Get<type::Bool>();
}

const type::I32* Manager::i32() {
    return Get<type::I32>();
}

const type::U32* Manager::u32() {
    return Get<type::U32>();
}

const type::F32* Manager::f32() {
    return Get<type::F32>();
}

const type::F16* Manager::f16() {
    return Get<type::F16>();
}

const type::AbstractFloat* Manager::AFloat() {
    return Get<type::AbstractFloat>();
}

const type::AbstractInt* Manager::AInt() {
    return Get<type::AbstractInt>();
}

const type::Vector* Manager::vec(const type::Type* inner, uint32_t size) {
    return Get<type::Vector>(inner, size);
}

const type::Vector* Manager::vec2(const type::Type* inner) {
    return vec(inner, 2);
}

const type::Vector* Manager::vec3(const type::Type* inner) {
    return vec(inner, 3);
}

const type::Vector* Manager::vec4(const type::Type* inner) {
    return vec(inner, 4);
}

const type::Matrix* Manager::mat(const type::Type* inner, uint32_t cols, uint32_t rows) {
    return Get<type::Matrix>(vec(inner, rows), cols);
}

const type::Matrix* Manager::mat2x2(const type::Type* inner) {
    return mat(inner, 2, 2);
}

const type::Matrix* Manager::mat2x3(const type::Type* inner) {
    return mat(inner, 2, 3);
}

const type::Matrix* Manager::mat2x4(const type::Type* inner) {
    return mat(inner, 2, 4);
}

const type::Matrix* Manager::mat3x2(const type::Type* inner) {
    return mat(inner, 3, 2);
}

const type::Matrix* Manager::mat3x3(const type::Type* inner) {
    return mat(inner, 3, 3);
}

const type::Matrix* Manager::mat3x4(const type::Type* inner) {
    return mat(inner, 3, 4);
}

const type::Matrix* Manager::mat4x2(const type::Type* inner) {
    return mat(inner, 4, 2);
}

const type::Matrix* Manager::mat4x3(const type::Type* inner) {
    return mat(inner, 4, 3);
}

const type::Matrix* Manager::mat4x4(const type::Type* inner) {
    return mat(inner, 4, 4);
}

const type::Array* Manager::array(const type::Type* elem_ty,
                                  uint32_t count,
                                  uint32_t stride /* = 0*/) {
    uint32_t implicit_stride = utils::RoundUp(elem_ty->Align(), elem_ty->Size());
    if (stride == 0) {
        stride = implicit_stride;
    }
    TINT_ASSERT(Type, stride >= implicit_stride);

    return Get<type::Array>(/* element type */ elem_ty,
                            /* element count */ Get<ConstantArrayCount>(count),
                            /* array alignment */ elem_ty->Align(),
                            /* array size */ count * stride,
                            /* element stride */ stride,
                            /* implicit stride */ implicit_stride);
}

const type::Array* Manager::runtime_array(const type::Type* elem_ty, uint32_t stride /* = 0 */) {
    if (stride == 0) {
        stride = elem_ty->Align();
    }
    return Get<type::Array>(
        /* element type */ elem_ty,
        /* element count */ Get<RuntimeArrayCount>(),
        /* array alignment */ elem_ty->Align(),
        /* array size */ stride,
        /* element stride */ stride,
        /* implicit stride */ elem_ty->Align());
}

const type::Pointer* Manager::pointer(const type::Type* subtype,
                                      builtin::AddressSpace address_space,
                                      builtin::Access access) {
    return Get<type::Pointer>(subtype, address_space, access);
}

}  // namespace tint::type
