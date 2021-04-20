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

#include "src/ast/sampler.h"

#include "src/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::Sampler);

namespace tint {
namespace ast {

std::ostream& operator<<(std::ostream& out, SamplerKind kind) {
  switch (kind) {
    case SamplerKind::kSampler:
      out << "sampler";
      break;
    case SamplerKind::kComparisonSampler:
      out << "comparison_sampler";
      break;
  }
  return out;
}

Sampler::Sampler(ProgramID program_id, const Source& source, SamplerKind kind)
    : Base(program_id, source), kind_(kind) {}

Sampler::Sampler(Sampler&&) = default;

Sampler::~Sampler() = default;

std::string Sampler::type_name() const {
  return std::string("__sampler_") +
         (kind_ == SamplerKind::kSampler ? "sampler" : "comparison");
}

std::string Sampler::FriendlyName(const SymbolTable&) const {
  return kind_ == SamplerKind::kSampler ? "sampler" : "sampler_comparison";
}

Sampler* Sampler::Clone(CloneContext* ctx) const {
  auto src = ctx->Clone(source());
  return ctx->dst->create<Sampler>(src, kind_);
}

}  // namespace ast
}  // namespace tint
