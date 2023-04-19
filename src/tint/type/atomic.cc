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

#include "src/tint/type/atomic.h"

#include "src/tint/debug.h"
#include "src/tint/diagnostic/diagnostic.h"
#include "src/tint/type/manager.h"
#include "src/tint/type/reference.h"
#include "src/tint/utils/hash.h"
#include "src/tint/utils/string_stream.h"

TINT_INSTANTIATE_TYPEINFO(tint::type::Atomic);

namespace tint::type {

Atomic::Atomic(const type::Type* subtype)
    : Base(utils::Hash(utils::TypeInfo::Of<Atomic>().full_hashcode, subtype),
           type::Flags{
               Flag::kCreationFixedFootprint,
               Flag::kFixedFootprint,
           }),
      subtype_(subtype) {
    TINT_ASSERT(AST, !subtype->Is<Reference>());
}

bool Atomic::Equals(const type::UniqueNode& other) const {
    if (auto* o = other.As<Atomic>()) {
        return o->subtype_ == subtype_;
    }
    return false;
}

std::string Atomic::FriendlyName() const {
    utils::StringStream out;
    out << "atomic<" << subtype_->FriendlyName() << ">";
    return out.str();
}

uint32_t Atomic::Size() const {
    return subtype_->Size();
}

uint32_t Atomic::Align() const {
    return subtype_->Align();
}

Atomic::~Atomic() = default;

Atomic* Atomic::Clone(CloneContext& ctx) const {
    auto* ty = subtype_->Clone(ctx);
    return ctx.dst.mgr->Get<Atomic>(ty);
}

}  // namespace tint::type
