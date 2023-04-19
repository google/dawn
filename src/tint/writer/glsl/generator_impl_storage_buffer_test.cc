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

#include "src/tint/number.h"
#include "src/tint/writer/glsl/test_helper.h"

#include "gmock/gmock.h"

using ::testing::HasSubstr;
using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::glsl {
namespace {

using GlslGeneratorImplTest_StorageBuffer = TestHelper;

void TestAlign(ProgramBuilder* ctx) {
    // struct Nephews {
    //   @align(256) huey  : f32;
    //   @align(256) dewey : f32;
    //   @align(256) louie : f32;
    // };
    // @group(0) @binding(0) var<storage, read_write> nephews : Nephews;
    auto* nephews = ctx->Structure(
        "Nephews", utils::Vector{
                       ctx->Member("huey", ctx->ty.f32(), utils::Vector{ctx->MemberAlign(256_i)}),
                       ctx->Member("dewey", ctx->ty.f32(), utils::Vector{ctx->MemberAlign(256_i)}),
                       ctx->Member("louie", ctx->ty.f32(), utils::Vector{ctx->MemberAlign(256_i)}),
                   });
    ctx->GlobalVar("nephews", ctx->ty.Of(nephews), builtin::AddressSpace::kStorage,
                   ctx->Binding(0_a), ctx->Group(0_a));
}

TEST_F(GlslGeneratorImplTest_StorageBuffer, Align) {
    TestAlign(this);

    GeneratorImpl& gen = Build();

    // TODO(crbug.com/tint/1421) offsets do not currently work on GLSL ES.
    // They will likely require manual padding.
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es

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

TEST_F(GlslGeneratorImplTest_StorageBuffer, Align_Desktop) {
    TestAlign(this);

    GeneratorImpl& gen = Build(Version(Version::Standard::kDesktop, 4, 4));
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 440

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
}  // namespace tint::writer::glsl
