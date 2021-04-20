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

#ifndef SRC_AST_SAMPLER_H_
#define SRC_AST_SAMPLER_H_

#include <string>

#include "src/ast/type.h"

namespace tint {
namespace ast {

/// The different kinds of samplers
enum class SamplerKind {
  /// A regular sampler
  kSampler,
  /// A comparison sampler
  kComparisonSampler
};

/// @param out the std::ostream to write to
/// @param kind the SamplerKind
/// @return the std::ostream so calls can be chained
std::ostream& operator<<(std::ostream& out, SamplerKind kind);

/// A sampler type.
class Sampler : public Castable<Sampler, Type> {
 public:
  /// Constructor
  /// @param program_id the identifier of the program that owns this node
  /// @param source the source of this node
  /// @param kind the kind of sampler
  Sampler(ProgramID program_id, const Source& source, SamplerKind kind);
  /// Move constructor
  Sampler(Sampler&&);
  ~Sampler() override;

  /// @returns the sampler type
  SamplerKind kind() const { return kind_; }

  /// @returns true if this is a comparison sampler
  bool IsComparison() const { return kind_ == SamplerKind::kComparisonSampler; }

  /// @returns the name for this type
  std::string type_name() const override;

  /// @param symbols the program's symbol table
  /// @returns the name for this type that closely resembles how it would be
  /// declared in WGSL.
  std::string FriendlyName(const SymbolTable& symbols) const override;

  /// Clones this type and all transitive types using the `CloneContext` `ctx`.
  /// @param ctx the clone context
  /// @return the newly cloned type
  Sampler* Clone(CloneContext* ctx) const override;

 private:
  SamplerKind const kind_;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_SAMPLER_H_
