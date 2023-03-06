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

#include "src/tint/sem/value_conversion.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::ValueConversion);

namespace tint::sem {

ValueConversion::ValueConversion(const type::Type* type,
                                 sem::Parameter* parameter,
                                 EvaluationStage stage)
    : Base(type, utils::Vector<sem::Parameter*, 1>{parameter}, stage, /* must_use */ true) {}

ValueConversion::~ValueConversion() = default;

}  // namespace tint::sem
