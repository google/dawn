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

#include "src/tint/writer/spirv/operand.h"

namespace tint::writer::spirv {

uint32_t OperandLength(const Operand& o) {
    if (auto* str = std::get_if<std::string>(&o)) {
        // SPIR-V always nul-terminates strings. The length is rounded up to a
        // multiple of 4 bytes with 0 bytes padding the end. Accounting for the
        // nul terminator is why '+ 4u' is used here instead of '+ 3u'.
        return static_cast<uint32_t>((str->length() + 4u) >> 2);
    }
    return 1;
}

}  // namespace tint::writer::spirv
