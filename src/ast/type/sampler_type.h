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

#ifndef SRC_AST_TYPE_SAMPLER_TYPE_H_
#define SRC_AST_TYPE_SAMPLER_TYPE_H_

#include <ostream>
#include <string>

#include "src/ast/type/type.h"

namespace tint {
namespace ast {
namespace type {

/// The different kinds of samplers
enum class SamplerKind {
  /// A regular sampler
  kSampler,
  /// A comparison sampler
  kComparisonSampler
};
std::ostream& operator<<(std::ostream& out, SamplerKind kind);

/// A sampler type.
class SamplerType : public Type {
 public:
  /// Constructor
  /// @param kind the kind of sampler
  explicit SamplerType(SamplerKind kind);
  /// Move constructor
  SamplerType(SamplerType&&);
  ~SamplerType() override;

  /// @returns true if the type is a sampler type
  bool IsSampler() const override;

  /// @returns the sampler type
  SamplerKind kind() const { return kind_; }

  /// @returns true if this is a comparison sampler
  bool IsComparison() const { return kind_ == SamplerKind::kComparisonSampler; }

  /// @returns the name for this type
  std::string type_name() const override;

 private:
  SamplerKind kind_ = SamplerKind::kSampler;
};

}  // namespace type
}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_TYPE_SAMPLER_TYPE_H_
