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

#include "src/tint/writer/glsl/test_helper.h"

namespace tint::writer::glsl {
namespace {

using GlslGeneratorImplTest = TestHelper;

TEST_F(GlslGeneratorImplTest, Generate) {
    Func("my_func", ast::VariableList{}, ty.void_(), ast::StatementList{}, ast::AttributeList{});

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(#version 310 es

void my_func() {
}

)");
}

TEST_F(GlslGeneratorImplTest, GenerateDesktop) {
    Func("my_func", ast::VariableList{}, ty.void_(), ast::StatementList{}, ast::AttributeList{});

    GeneratorImpl& gen = Build(Version(Version::Standard::kDesktop, 4, 4));

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(#version 440

void my_func() {
}

)");
}

TEST_F(GlslGeneratorImplTest, GenerateSampleIndexES) {
    Global("gl_SampleID", ty.i32(),
           ast::AttributeList{Builtin(ast::Builtin::kSampleIndex),
                              Disable(ast::DisabledValidation::kIgnoreStorageClass)},
           ast::StorageClass::kInput);
    Func("my_func", {}, ty.i32(), ast::StatementList{Return(Expr("gl_SampleID"))});

    GeneratorImpl& gen = Build(Version(Version::Standard::kES, 3, 1));

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(#version 310 es
#extension GL_OES_sample_variables : require

int my_func() {
  return gl_SampleID;
}

)");
}

TEST_F(GlslGeneratorImplTest, GenerateSampleIndexDesktop) {
    Global("gl_SampleID", ty.i32(),
           ast::AttributeList{Builtin(ast::Builtin::kSampleIndex),
                              Disable(ast::DisabledValidation::kIgnoreStorageClass)},
           ast::StorageClass::kInput);
    Func("my_func", {}, ty.i32(), ast::StatementList{Return(Expr("gl_SampleID"))});

    GeneratorImpl& gen = Build(Version(Version::Standard::kDesktop, 4, 4));

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(#version 440

int my_func() {
  return gl_SampleID;
}

)");
}

}  // namespace
}  // namespace tint::writer::glsl
