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

#include "src/tint/lang/glsl/writer/ast_printer/helper_test.h"

#include "gmock/gmock.h"

namespace tint::glsl::writer {
namespace {

using GlslASTPrinterTest = TestHelper;

TEST_F(GlslASTPrinterTest, InvalidProgram) {
    Diagnostics().add_error(diag::System::Writer, "make the program invalid");
    ASSERT_FALSE(IsValid());
    auto program = std::make_unique<Program>(resolver::Resolve(*this));
    ASSERT_FALSE(program->IsValid());
    auto result = Generate(program.get(), Options{}, "");
    EXPECT_FALSE(result);
    EXPECT_EQ(result.Failure(), "input program is not valid");
}

TEST_F(GlslASTPrinterTest, Generate) {
    Func("my_func", tint::Empty, ty.void_(), tint::Empty);

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es

void my_func() {
}

)");
}

TEST_F(GlslASTPrinterTest, GenerateDesktop) {
    Func("my_func", tint::Empty, ty.void_(), tint::Empty);

    ASTPrinter& gen = Build(Version(Version::Standard::kDesktop, 4, 4));
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 440

void my_func() {
}

)");
}

TEST_F(GlslASTPrinterTest, GenerateSampleIndexES) {
    GlobalVar("gl_SampleID", ty.i32(),
              Vector{
                  Builtin(builtin::BuiltinValue::kSampleIndex),
                  Disable(ast::DisabledValidation::kIgnoreAddressSpace),
              },
              builtin::AddressSpace::kIn);
    Func("my_func", tint::Empty, ty.i32(),
         Vector{
             Return(Expr("gl_SampleID")),
         });

    ASTPrinter& gen = Build(Version(Version::Standard::kES, 3, 1));
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es
#extension GL_OES_sample_variables : require

int my_func() {
  return gl_SampleID;
}

)");
}

TEST_F(GlslASTPrinterTest, GenerateSampleIndexDesktop) {
    GlobalVar("gl_SampleID", ty.i32(),
              Vector{
                  Builtin(builtin::BuiltinValue::kSampleIndex),
                  Disable(ast::DisabledValidation::kIgnoreAddressSpace),
              },
              builtin::AddressSpace::kIn);
    Func("my_func", tint::Empty, ty.i32(),
         Vector{
             Return(Expr("gl_SampleID")),
         });

    ASTPrinter& gen = Build(Version(Version::Standard::kDesktop, 4, 4));
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 440

int my_func() {
  return gl_SampleID;
}

)");
}

}  // namespace
}  // namespace tint::glsl::writer
