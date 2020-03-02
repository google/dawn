
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

#ifndef SRC_SOURCE_H_
#define SRC_SOURCE_H_

#include <stddef.h>

namespace tint {

/// Represents a line and column position
struct Source {
  /// The line the token appeared on
  size_t line = 0;
  /// The column the token appeared in
  size_t column = 0;
};

}  // namespace tint

#endif  // SRC_SOURCE_H_
