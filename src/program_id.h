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

#ifndef SRC_PROGRAM_ID_H_
#define SRC_PROGRAM_ID_H_

#include <stdint.h>

namespace tint {

/// A ProgramID is a unique identifier of a Program.
/// ProgramID can be used to ensure that objects referenced by the Program are
/// owned exclusively by that Program and have accidentally not leaked from
/// another Program.
class ProgramID {
 public:
  /// Constructor
  ProgramID();

  /// @returns a new ProgramID
  static ProgramID New();

  /// Equality operator
  /// @param rhs the other ProgramID
  /// @returns true if the ProgramIDs are equal
  bool operator==(const ProgramID& rhs) const { return val == rhs.val; }

  /// Inequality operator
  /// @param rhs the other ProgramID
  /// @returns true if the ProgramIDs are not equal
  bool operator!=(const ProgramID& rhs) const { return val != rhs.val; }

 private:
  explicit ProgramID(uint32_t);

  uint32_t val = 0;
};

}  // namespace tint

#endif  // SRC_PROGRAM_ID_H_
