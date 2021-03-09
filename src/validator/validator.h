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

#ifndef SRC_VALIDATOR_VALIDATOR_H_
#define SRC_VALIDATOR_VALIDATOR_H_

#include <string>

#include "src/program.h"

namespace tint {

/// Determines if the program is complete and valid
class Validator {
 public:
  /// Constructor
  Validator();
  ~Validator();

  /// Runs the validator
  /// @param program the program to validate
  /// @returns true if the validation was successful
  bool Validate(const Program* program);

  /// @returns error messages from the validator
  std::string error() {
    diag::Formatter formatter{{false, false, false, false}};
    return formatter.format(diags_);
  }
  /// @returns true if an error was encountered
  bool has_error() const { return diags_.contains_errors(); }
  /// @returns the full list of diagnostic messages.
  const diag::List& diagnostics() const { return diags_; }

 private:
  diag::List diags_;
};

}  // namespace tint

#endif  // SRC_VALIDATOR_VALIDATOR_H_
