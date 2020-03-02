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

#ifndef SRC_TYPE_DETERMINER_H_
#define SRC_TYPE_DETERMINER_H_

#include <string>

#include "src/ast/module.h"

namespace tint {

/// Determines types for all items in the given tint module
class TypeDeterminer {
 public:
  /// Constructor
  TypeDeterminer();
  ~TypeDeterminer();

  /// Runs the type determiner
  /// @param module the module to update with typing information
  /// @returns true if the type determiner was successful
  bool Determine(ast::Module* module);

  /// @returns error messages from the type determiner
  const std::string& error() { return error_; }

 private:
  std::string error_;
};

}  // namespace tint

#endif  // SRC_TYPE_DETERMINER_H_
