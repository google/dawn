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

#ifndef SRC_TINT_RESOLVER_CONST_EVAL_H_
#define SRC_TINT_RESOLVER_CONST_EVAL_H_

#include <stddef.h>

// Forward declarations
namespace tint {
class ProgramBuilder;
}  // namespace tint

// Forward declarations
namespace tint::sem {
class Constant;
}  // namespace tint::sem

namespace tint::resolver::const_eval {

/// Typedef for a constant evaluation function
using Function = sem::Constant(ProgramBuilder& builder, const sem::Constant* args, size_t num_args);

}  // namespace tint::resolver::const_eval

#endif  // SRC_TINT_RESOLVER_CONST_EVAL_H_
