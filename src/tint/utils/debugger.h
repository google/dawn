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

#ifndef SRC_TINT_UTILS_DEBUGGER_H_
#define SRC_TINT_UTILS_DEBUGGER_H_

namespace tint::debugger {

/// If debugger is attached and the `TINT_ENABLE_BREAK_IN_DEBUGGER` preprocessor
/// macro is defined for `debugger.cc`, calling `Break()` will cause the
/// debugger to break at the call site.
void Break();

}  // namespace tint::debugger

#endif  // SRC_TINT_UTILS_DEBUGGER_H_
