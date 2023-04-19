// Copyright 2021 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0(the "License");
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

#ifndef SRC_TINT_SEM_VALUE_CONVERSION_H_
#define SRC_TINT_SEM_VALUE_CONVERSION_H_

#include "src/tint/sem/call_target.h"

namespace tint::sem {

/// ValueConversion is the CallTarget for a value conversion (cast).
class ValueConversion final : public utils::Castable<ValueConversion, CallTarget> {
  public:
    /// Constructor
    /// @param type the target type of the cast
    /// @param parameter the type cast parameter
    /// @param stage the earliest evaluation stage for the expression
    ValueConversion(const type::Type* type, sem::Parameter* parameter, EvaluationStage stage);

    /// Destructor
    ~ValueConversion() override;

    /// @returns the cast source type
    const type::Type* Source() const { return Parameters()[0]->Type(); }

    /// @returns the cast target type
    const type::Type* Target() const { return ReturnType(); }
};

}  // namespace tint::sem

#endif  // SRC_TINT_SEM_VALUE_CONVERSION_H_
