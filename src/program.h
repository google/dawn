// Copyright 2021 The Tint Authors.
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

#ifndef SRC_PROGRAM_H_
#define SRC_PROGRAM_H_

#include <string>

#include "src/ast/module.h"

namespace tint {

/// Program is (currently) a simple wrapper of to ast::Module.
/// This wrapper is used as a stepping stone to having Dawn use tint::Program
/// instead of tint::ast::Module.
class Program {
 public:
  /// The wrapped module
  ast::Module module;

  /// @returns true if all required fields in the module are present.
  bool IsValid() const { return module.IsValid(); }

  /// @return a deep copy of this program
  Program Clone() const { return Program{module.Clone()}; }
};

}  // namespace tint

#endif  // SRC_PROGRAM_H_
