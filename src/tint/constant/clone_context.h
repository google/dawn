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

#ifndef SRC_TINT_CONSTANT_CLONE_CONTEXT_H_
#define SRC_TINT_CONSTANT_CLONE_CONTEXT_H_

#include "src/tint/type/clone_context.h"
#include "src/tint/utils/block_allocator.h"

// Forward Declarations
namespace tint::constant {
class Value;
}  // namespace tint::constant

namespace tint::constant {

/// Context information for cloning of constants
struct CloneContext {
    /// The context for cloning type information
    type::CloneContext type_ctx;

    /// Destination information
    struct {
        /// The constant allocator
        utils::BlockAllocator<constant::Value>* constants;
    } dst;
};

}  // namespace tint::constant

#endif  // SRC_TINT_CONSTANT_CLONE_CONTEXT_H_
