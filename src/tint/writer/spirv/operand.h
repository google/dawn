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

#ifndef SRC_TINT_WRITER_SPIRV_OPERAND_H_
#define SRC_TINT_WRITER_SPIRV_OPERAND_H_

#include <cstring>
#include <string>
#include <variant>
#include <vector>

#include "src/tint/utils/hash.h"

namespace tint::writer::spirv {

/// A single SPIR-V instruction operand
using Operand = std::variant<uint32_t, float, std::string>;

// Helper for returning an uint32_t Operand with the provided integer value.
template <typename T>
inline Operand U32Operand(T val) {
    return Operand{static_cast<uint32_t>(val)};
}

/// @returns the number of uint32_t's needed for this operand
uint32_t OperandLength(const Operand& o);

/// A list of operands
using OperandList = std::vector<Operand>;

using OperandListKey = utils::UnorderedKeyWrapper<OperandList>;

}  // namespace tint::writer::spirv

#endif  // SRC_TINT_WRITER_SPIRV_OPERAND_H_
