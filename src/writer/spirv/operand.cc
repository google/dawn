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

#include "src/writer/spirv/operand.h"

#include <cmath>

namespace tint {
namespace writer {
namespace spirv {

// static
Operand Operand::Float(float val) {
  Operand o(Kind::kFloat);
  o.set_float(val);
  return o;
}

// static
Operand Operand::Int(uint32_t val) {
  Operand o(Kind::kInt);
  o.set_int(val);
  return o;
}

// static
Operand Operand::String(const std::string& val) {
  Operand o(Kind::kString);
  o.set_string(val);
  return o;
}

Operand::Operand(Kind kind) : kind_(kind) {}

Operand::~Operand() = default;

uint32_t Operand::length() const {
  uint32_t val = 0;
  switch (kind_) {
    case Kind::kFloat:
    case Kind::kInt:
      val = 1;
      break;
    case Kind::kString:
      // SPIR-V always nul-terminates strings. The length is rounded up to a
      // multiple of 4 bytes with 0 bytes padding the end. Accounting for the
      // nul terminator is why '+ 4u' is used here instead of '+ 3u'.
      val = static_cast<uint32_t>((str_val_.length() + 4u) >> 2);
      break;
  }
  return val;
}

}  // namespace spirv
}  // namespace writer
}  // namespace tint
