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

#ifndef SRC_TINT_TYPE_SAMPLER_KIND_H_
#define SRC_TINT_TYPE_SAMPLER_KIND_H_

#include "src/tint/utils/string_stream.h"

namespace tint::type {

/// The different kinds of samplers
enum class SamplerKind {
    /// A regular sampler
    kSampler,
    /// A comparison sampler
    kComparisonSampler
};

/// @param out the stream to write to
/// @param kind the SamplerKind
/// @return the stream so calls can be chained
utils::StringStream& operator<<(utils::StringStream& out, SamplerKind kind);

}  // namespace tint::type

#endif  // SRC_TINT_TYPE_SAMPLER_KIND_H_
