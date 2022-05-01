// Copyright 2021 The Tint Authors.
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

#include "src/tint/writer/text_generator.h"

#include "gtest/gtest.h"

namespace tint::writer {
namespace {

TEST(TextGeneratorTest, UniqueIdentifier) {
    Program program(ProgramBuilder{});

    TextGenerator gen(&program);

    ASSERT_EQ(gen.UniqueIdentifier("ident"), "ident");
    ASSERT_EQ(gen.UniqueIdentifier("ident"), "ident_1");
}

TEST(TextGeneratorTest, UniqueIdentifier_ConflictWithExisting) {
    ProgramBuilder builder;
    builder.Symbols().Register("ident_1");
    builder.Symbols().Register("ident_2");
    Program program(std::move(builder));

    TextGenerator gen(&program);

    ASSERT_EQ(gen.UniqueIdentifier("ident"), "ident");
    ASSERT_EQ(gen.UniqueIdentifier("ident"), "ident_3");
    ASSERT_EQ(gen.UniqueIdentifier("ident"), "ident_4");
    ASSERT_EQ(gen.UniqueIdentifier("ident"), "ident_5");
}

}  // namespace
}  // namespace tint::writer
