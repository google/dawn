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

#include "src/ast/access_control.h"

namespace tint {
namespace ast {

std::ostream& operator<<(std::ostream& out, AccessControl access) {
  switch (access) {
    case ast::AccessControl::kReadOnly: {
      out << "read_only";
      break;
    }
    case ast::AccessControl::kReadWrite: {
      out << "read_write";
      break;
    }
    case ast::AccessControl::kWriteOnly: {
      out << "write_only";
      break;
    }
  }
  return out;
}

}  // namespace ast
}  // namespace tint
