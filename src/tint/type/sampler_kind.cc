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

#include "src/tint/type/sampler_kind.h"

namespace tint::type {

utils::StringStream& operator<<(utils::StringStream& out, SamplerKind kind) {
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

}  // namespace tint::type
