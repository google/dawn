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

#include "src/tint/writer/ast_text_generator.h"

#include <algorithm>
#include <limits>

#include "src/tint/utils/map.h"

namespace tint::writer {

ASTTextGenerator::ASTTextGenerator(const Program* program)
    : program_(program), builder_(ProgramBuilder::Wrap(program)) {}

ASTTextGenerator::~ASTTextGenerator() = default;

std::string ASTTextGenerator::UniqueIdentifier(const std::string& prefix) {
    return builder_.Symbols().New(prefix).Name();
}

std::string ASTTextGenerator::StructName(const type::Struct* s) {
    auto name = s->Name().Name();
    if (name.size() > 1 && name[0] == '_' && name[1] == '_') {
        name = utils::GetOrCreate(builtin_struct_names_, s,
                                  [&] { return UniqueIdentifier(name.substr(2)); });
    }
    return name;
}

}  // namespace tint::writer
