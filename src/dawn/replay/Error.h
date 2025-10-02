// Copyright 2025 The Dawn & Tint Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS-IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SRC_DAWN_REPLAY_ERROR_H_
#define SRC_DAWN_REPLAY_ERROR_H_

#include "dawn/native/Error.h"

namespace dawn::replay {

// Pull these into replay for now. We can replace them later if needed.
using MaybeError = dawn::native::MaybeError;
using InternalErrorType = dawn::native::InternalErrorType;
using MaybeError = dawn::native::MaybeError;
template <typename T>
using ResultOrError = dawn::native::ResultOrError<T>;

}  // namespace dawn::replay

#endif  // SRC_DAWN_REPLAY_ERROR_H_
