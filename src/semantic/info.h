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

#ifndef SRC_SEMANTIC_INFO_H_
#define SRC_SEMANTIC_INFO_H_

namespace tint {

namespace semantic {

/// Info will hold all the resolved semantic information for a Program.
class Info {
 public:
  /// Constructor
  Info();

  /// Move constructor
  Info(Info&&);

  /// Destructor
  ~Info();

  /// Move assignment operator
  /// @param rhs the Program to move
  /// @return this Program
  Info& operator=(Info&& rhs);
};

}  // namespace semantic
}  // namespace tint

#endif  // SRC_SEMANTIC_INFO_H_
