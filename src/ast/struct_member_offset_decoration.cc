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

#include "src/ast/struct_member_offset_decoration.h"

namespace tint {
namespace ast {

StructMemberOffsetDecoration::StructMemberOffsetDecoration(uint32_t offset)
    : offset_(offset) {}

bool StructMemberOffsetDecoration::IsOffset() const {
  return true;
}

StructMemberOffsetDecoration::~StructMemberOffsetDecoration() = default;

std::string StructMemberOffsetDecoration::to_str() const {
  return "offset " + std::to_string(offset_);
}

}  // namespace ast
}  // namespace tint
