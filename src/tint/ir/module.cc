// Copyright 2022 The Tint Authors.
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

#include "src/tint/ir/module.h"

#include "src/tint/ir/builder_impl.h"
#include "src/tint/program.h"

namespace tint::ir {

// static
Module::Result Module::FromProgram(const Program* program) {
    if (!program->IsValid()) {
        return Result{std::string("input program is not valid")};
    }

    BuilderImpl b(program);
    auto r = b.Build();
    if (!r) {
        return b.error();
    }

    return Result{r.Move()};
}

Module::Module() = default;

Module::Module(Module&&) = default;

Module::~Module() = default;

Module& Module::operator=(Module&&) = default;

const Program* Module::ToProgram() const {
    return nullptr;
}

}  // namespace tint::ir
