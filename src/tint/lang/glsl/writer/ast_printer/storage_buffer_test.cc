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

#include "src/tint/lang/core/number.h"
#include "src/tint/lang/glsl/writer/ast_printer/helper_test.h"

#include "gmock/gmock.h"

using ::testing::HasSubstr;
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::glsl::writer {
namespace {

using GlslASTPrinterTest_StorageBuffer = TestHelper;

void TestAlign(ProgramBuilder* ctx) {
    // struct Nephews {
    //   @align(256) huey  : f32;
    //   @align(256) dewey : f32;
    //   @align(256) louie : f32;
    // };
    // @group(0) @binding(0) var<storage, read_write> nephews : Nephews;
    auto* nephews = ctx->Structure(
        "Nephews", Vector{
                       ctx->Member("huey", ctx->ty.f32(), Vector{ctx->MemberAlign(256_i)}),
                       ctx->Member("dewey", ctx->ty.f32(), Vector{ctx->MemberAlign(256_i)}),
                       ctx->Member("louie", ctx->ty.f32(), Vector{ctx->MemberAlign(256_i)}),
                   });
    ctx->GlobalVar("nephews", ctx->ty.Of(nephews), core::AddressSpace::kStorage, ctx->Binding(0_a),
                   ctx->Group(0_a));
}

TEST_F(GlslASTPrinterTest_StorageBuffer, Align) {
    TestAlign(this);

    ASTPrinter& gen = Build();

    // TODO(crbug.com/tint/1421) offsets do not currently work on GLSL ES.
    // They will likely require manual padding.
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es

struct Nephews {
  float huey;
  float dewey;
  float louie;
};

layout(binding = 0, std430) buffer Nephews_ssbo {
  float huey;
  float dewey;
  float louie;
} nephews;

)");
}

TEST_F(GlslASTPrinterTest_StorageBuffer, Align_Desktop) {
    TestAlign(this);

    ASTPrinter& gen = Build(Version(Version::Standard::kDesktop, 4, 4));
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 440

struct Nephews {
  float huey;
  float dewey;
  float louie;
};

layout(binding = 0, std430) buffer Nephews_ssbo {
  float huey;
  float dewey;
  float louie;
} nephews;

)");
}

}  // namespace
}  // namespace tint::glsl::writer
