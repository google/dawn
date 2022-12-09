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

#ifndef SRC_TINT_SEM_EVALUATION_STAGE_H_
#define SRC_TINT_SEM_EVALUATION_STAGE_H_

#include <algorithm>
#include <initializer_list>

namespace tint::sem {

/// The earliest point in time that an expression can be evaluated
enum class EvaluationStage {
    /// Expression will not be evaluated
    kNotEvaluated,
    /// Expression can be evaluated at shader creation time
    kConstant,
    /// Expression can be evaluated at pipeline creation time
    kOverride,
    /// Expression can be evaluated at runtime
    kRuntime,
};

/// @returns true if stage `a` comes earlier than stage `b`
inline bool operator<(EvaluationStage a, EvaluationStage b) {
    return static_cast<int>(a) < static_cast<int>(b);
}

/// @returns true if stage `a` comes later than stage `b`
inline bool operator>(EvaluationStage a, EvaluationStage b) {
    return static_cast<int>(a) > static_cast<int>(b);
}

/// @param stages a list of EvaluationStage.
/// @returns the earliest stage supported by all the provided stages
inline EvaluationStage EarliestStage(std::initializer_list<EvaluationStage> stages) {
    auto earliest = EvaluationStage::kNotEvaluated;
    for (auto stage : stages) {
        earliest = std::max(stage, earliest);
    }
    return static_cast<EvaluationStage>(earliest);
}

template <typename... ARGS>
inline EvaluationStage EarliestStage(ARGS... args) {
    return EarliestStage({args...});
}

}  // namespace tint::sem

#endif  // SRC_TINT_SEM_EVALUATION_STAGE_H_
