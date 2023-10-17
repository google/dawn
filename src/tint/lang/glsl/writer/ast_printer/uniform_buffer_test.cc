// Copyright 2022 The Dawn & Tint Authors
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

using ::testing::HasSubstr;

namespace tint::glsl::writer {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT

using GlslASTPrinterTest_UniformBuffer = TestHelper;

TEST_F(GlslASTPrinterTest_UniformBuffer, Simple) {
    auto* simple = Structure("Simple", Vector{Member("member", ty.f32())});
    GlobalVar("simple", ty.Of(simple), core::AddressSpace::kUniform, Group(0_a), Binding(0_a));

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es

struct Simple {
  float member;
};

layout(binding = 0, std140) uniform Simple_ubo {
  float member;
} simple;

)");
}

TEST_F(GlslASTPrinterTest_UniformBuffer, Simple_Desktop) {
    auto* simple = Structure("Simple", Vector{Member("member", ty.f32())});
    GlobalVar("simple", ty.Of(simple), core::AddressSpace::kUniform, Group(0_a), Binding(0_a));

    ASTPrinter& gen = Build(Version(Version::Standard::kDesktop, 4, 4));
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 440

struct Simple {
  float member;
};

layout(binding = 0, std140) uniform Simple_ubo {
  float member;
} simple;

)");
}

}  // namespace
}  // namespace tint::glsl::writer
