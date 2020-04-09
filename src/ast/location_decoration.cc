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

#include "src/ast/location_decoration.h"

namespace tint {
namespace ast {

LocationDecoration::LocationDecoration(uint32_t val) : value_(val) {}

LocationDecoration::~LocationDecoration() = default;

bool LocationDecoration::IsLocation() const {
  return true;
}

void LocationDecoration::to_str(std::ostream& out) const {
  out << "LocationDecoration{" << value_ << "}" << std::endl;
}

}  // namespace ast
}  // namespace tint
