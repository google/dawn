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

#include "src/tint/symbol.h"

#include <utility>

namespace tint {

Symbol::Symbol() = default;

Symbol::Symbol(uint32_t val, tint::ProgramID pid, std::string_view name)
    : val_(val), program_id_(pid), name_(name) {
    DetermineBuiltinType();
}

Symbol::Symbol(const Symbol& o) = default;

Symbol::Symbol(Symbol&& o) = default;

Symbol::~Symbol() = default;

Symbol& Symbol::operator=(const Symbol& o) = default;

Symbol& Symbol::operator=(Symbol&& o) = default;

bool Symbol::operator==(const Symbol& other) const {
    TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(Symbol, program_id_, other.program_id_);
    return val_ == other.val_;
}

bool Symbol::operator!=(const Symbol& other) const {
    TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(Symbol, program_id_, other.program_id_);
    return val_ != other.val_;
}

bool Symbol::operator<(const Symbol& other) const {
    TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(Symbol, program_id_, other.program_id_);
    return val_ < other.val_;
}

std::string Symbol::to_str() const {
    return "$" + std::to_string(val_);
}

std::string_view Symbol::NameView() const {
    return name_;
}

std::string Symbol::Name() const {
    return std::string(name_);
}

void Symbol::DetermineBuiltinType() {
    if (auto builtin_fn = builtin::ParseFunction(name_); builtin_fn != builtin::Function::kNone) {
        builtin_type_ = BuiltinType::kFunction;
        builtin_value_ = builtin_fn;
        return;
    }
    if (auto builtin_ty = builtin::ParseBuiltin(name_);
        builtin_ty != builtin::Builtin::kUndefined) {
        builtin_type_ = BuiltinType::kBuiltin;
        builtin_value_ = builtin_ty;
        return;
    }
    if (auto builtin_val = builtin::ParseBuiltinValue(name_);
        builtin_val != builtin::BuiltinValue::kUndefined) {
        builtin_type_ = BuiltinType::kBuiltinValue;
        builtin_value_ = builtin_val;
        return;
    }
    if (auto addr = builtin::ParseAddressSpace(name_); addr != builtin::AddressSpace::kUndefined) {
        builtin_type_ = BuiltinType::kAddressSpace;
        builtin_value_ = addr;
        return;
    }
    if (auto fmt = builtin::ParseTexelFormat(name_); fmt != builtin::TexelFormat::kUndefined) {
        builtin_type_ = BuiltinType::kTexelFormat;
        builtin_value_ = fmt;
        return;
    }
    if (auto access = builtin::ParseAccess(name_); access != builtin::Access::kUndefined) {
        builtin_type_ = BuiltinType::kAccess;
        builtin_value_ = access;
        return;
    }
    if (auto i_type = builtin::ParseInterpolationType(name_);
        i_type != builtin::InterpolationType::kUndefined) {
        builtin_type_ = BuiltinType::kInterpolationType;
        builtin_value_ = i_type;
        return;
    }
    if (auto i_smpl = builtin::ParseInterpolationSampling(name_);
        i_smpl != builtin::InterpolationSampling::kUndefined) {
        builtin_type_ = BuiltinType::kInterpolationSampling;
        builtin_value_ = i_smpl;
        return;
    }
}

}  // namespace tint
