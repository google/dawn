// Copyright 2020 The Tint Authors.
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

#ifndef SRC_CONTEXT_H_
#define SRC_CONTEXT_H_

#include <assert.h>

#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

#include "src/namer.h"

namespace tint {

namespace ast {
class Node;
}

/// Context object for Tint. Holds various global resources used through
/// the system.
class Context {
 public:
  /// Constructor
  Context();
  /// Constructor
  /// @param namer the namer to set into the context
  explicit Context(std::unique_ptr<Namer> namer);
  /// Destructor
  ~Context();
};

}  // namespace tint

#endif  // SRC_CONTEXT_H_
