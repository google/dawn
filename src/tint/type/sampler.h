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

#ifndef SRC_TINT_TYPE_SAMPLER_H_
#define SRC_TINT_TYPE_SAMPLER_H_

#include <string>

#include "src/tint/type/sampler_kind.h"
#include "src/tint/type/type.h"

namespace tint::type {

/// A sampler type.
class Sampler final : public utils::Castable<Sampler, Type> {
  public:
    /// Constructor
    /// @param kind the kind of sampler
    explicit Sampler(SamplerKind kind);

    /// Destructor
    ~Sampler() override;

    /// @param other the other node to compare against
    /// @returns true if the this type is equal to @p other
    bool Equals(const UniqueNode& other) const override;

    /// @returns the sampler type
    SamplerKind kind() const { return kind_; }

    /// @returns true if this is a comparison sampler
    bool IsComparison() const { return kind_ == SamplerKind::kComparisonSampler; }

    /// @returns the name for this type that closely resembles how it would be
    /// declared in WGSL.
    std::string FriendlyName() const override;

    /// @param ctx the clone context
    /// @returns a clone of this type
    Sampler* Clone(CloneContext& ctx) const override;

  private:
    SamplerKind const kind_;
};

}  // namespace tint::type

#endif  // SRC_TINT_TYPE_SAMPLER_H_
