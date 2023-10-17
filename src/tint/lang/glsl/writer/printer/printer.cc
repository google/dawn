// Copyright 2023 The Tint Authors.
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

#include "src/tint/lang/glsl/writer/printer/printer.h"

#include <utility>

#include "src/tint/lang/core/ir/validator.h"

using namespace tint::core::fluent_types;  // NOLINT

namespace tint::glsl::writer {

Printer::Printer(core::ir::Module& module) : ir_(module) {}

Printer::~Printer() = default;

tint::Result<SuccessType> Printer::Generate() {
    auto valid = core::ir::ValidateAndDumpIfNeeded(ir_, "GLSL writer");
    if (!valid) {
        return std::move(valid.Failure());
    }

    return Success;
}

std::string Printer::Result() const {
    StringStream ss;
    ss << preamble_buffer_.String() << '\n' << main_buffer_.String();
    return ss.str();
}

}  // namespace tint::glsl::writer
