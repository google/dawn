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

#include "src/tint/utils/result/result.h"

namespace tint {

Failure::Failure() = default;

Failure::Failure(std::string_view err) {
    reason.add_error(diag::System::Unknown, err, Source{});
}

Failure::Failure(diag::List diagnostics) : reason(diagnostics) {}

}  // namespace tint
