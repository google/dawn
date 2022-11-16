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

#ifndef SRC_TINT_IR_DEBUG_H_
#define SRC_TINT_IR_DEBUG_H_

#include <string>

#include "src/tint/ir/module.h"

namespace tint::ir {

/// Helper class to debug IR.
class Debug {
  public:
    /// Returns the module as a dot graph
    /// @param mod the module to emit
    /// @returns the dot graph for the given module
    static std::string AsDotGraph(const Module* mod);

    /// Returns the module as a string
    /// @param mod the module to emit
    /// @returns the string representation of the module
    static std::string AsString(const Module* mod);
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_DEBUG_H_
