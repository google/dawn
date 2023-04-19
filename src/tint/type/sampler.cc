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

#include "src/tint/type/sampler.h"

#include "src/tint/type/manager.h"
#include "src/tint/utils/hash.h"

TINT_INSTANTIATE_TYPEINFO(tint::type::Sampler);

namespace tint::type {

Sampler::Sampler(SamplerKind kind)
    : Base(utils::Hash(utils::TypeInfo::Of<Sampler>().full_hashcode, kind), type::Flags{}),
      kind_(kind) {}

Sampler::~Sampler() = default;

bool Sampler::Equals(const UniqueNode& other) const {
    if (auto* o = other.As<Sampler>()) {
        return o->kind_ == kind_;
    }
    return false;
}

std::string Sampler::FriendlyName() const {
    return kind_ == SamplerKind::kSampler ? "sampler" : "sampler_comparison";
}

Sampler* Sampler::Clone(CloneContext& ctx) const {
    return ctx.dst.mgr->Get<Sampler>(kind_);
}

}  // namespace tint::type
