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

#ifndef SRC_SEM_SAMPLER_TYPE_H_
#define SRC_SEM_SAMPLER_TYPE_H_

#include <string>

#include "src/ast/sampler.h"
#include "src/sem/type.h"

namespace tint {
namespace sem {

/// A sampler type.
class Sampler : public Castable<Sampler, Type> {
 public:
  /// Constructor
  /// @param kind the kind of sampler
  explicit Sampler(ast::SamplerKind kind);
  /// Move constructor
  Sampler(Sampler&&);
  ~Sampler() override;

  /// @returns the sampler type
  ast::SamplerKind kind() const { return kind_; }

  /// @returns true if this is a comparison sampler
  bool IsComparison() const {
    return kind_ == ast::SamplerKind::kComparisonSampler;
  }

  /// @returns the name for this type
  std::string type_name() const override;

  /// @param symbols the program's symbol table
  /// @returns the name for this type that closely resembles how it would be
  /// declared in WGSL.
  std::string FriendlyName(const SymbolTable& symbols) const override;

 private:
  ast::SamplerKind const kind_;
};

}  // namespace sem
}  // namespace tint

#endif  // SRC_SEM_SAMPLER_TYPE_H_
