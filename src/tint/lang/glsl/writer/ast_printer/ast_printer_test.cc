// Copyright 2021 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/tint/lang/glsl/writer/ast_printer/helper_test.h"

#include "gmock/gmock.h"

namespace tint::glsl::writer {
namespace {

using GlslASTPrinterTest = TestHelper;

TEST_F(GlslASTPrinterTest, InvalidProgram) {
    Diagnostics().AddError(Source{}) << "make the program invalid";
    ASSERT_FALSE(IsValid());
    auto program = resolver::Resolve(*this);
    ASSERT_FALSE(program.IsValid());
    auto result = Generate(program, Options{}, "");
    EXPECT_NE(result, Success);
    EXPECT_EQ(result.Failure().reason.Str(), "error: make the program invalid");
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
                  Builtin(core::BuiltinValue::kSampleIndex),
                  Disable(ast::DisabledValidation::kIgnoreAddressSpace),
              },
              core::AddressSpace::kIn);
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
                  Builtin(core::BuiltinValue::kSampleIndex),
                  Disable(ast::DisabledValidation::kIgnoreAddressSpace),
              },
              core::AddressSpace::kIn);
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

TEST_F(GlslASTPrinterTest, UnsupportedExtension) {
    Enable(Source{{12, 34}}, wgsl::Extension::kChromiumInternalRelaxedUniformLayout);

    ASTPrinter& gen = Build();

    ASSERT_FALSE(gen.Generate());
    EXPECT_EQ(
        gen.Diagnostics().Str(),
        R"(12:34 error: GLSL backend does not support extension 'chromium_internal_relaxed_uniform_layout')");
}

TEST_F(GlslASTPrinterTest, RequiresDirective) {
    Require(wgsl::LanguageFeature::kReadonlyAndReadwriteStorageTextures);

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(#version 310 es

)");
}

}  // namespace
}  // namespace tint::glsl::writer
