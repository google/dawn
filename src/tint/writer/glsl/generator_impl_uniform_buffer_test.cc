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

#include "src/tint/writer/glsl/test_helper.h"

#include "gmock/gmock.h"

using ::testing::HasSubstr;

namespace tint::writer::glsl {
namespace {

using namespace tint::number_suffixes;  // NOLINT

using GlslGeneratorImplTest_UniformBuffer = TestHelper;

TEST_F(GlslGeneratorImplTest_UniformBuffer, Simple) {
    auto* simple = Structure("Simple", utils::Vector{Member("member", ty.f32())});
    GlobalVar("simple", ty.Of(simple), builtin::AddressSpace::kUniform, Group(0_a), Binding(0_a));

    GeneratorImpl& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es

struct Simple {
  float member;
};

layout(binding = 0, std140) uniform Simple_ubo {
  float member;
} simple;

)");
}

TEST_F(GlslGeneratorImplTest_UniformBuffer, Simple_Desktop) {
    auto* simple = Structure("Simple", utils::Vector{Member("member", ty.f32())});
    GlobalVar("simple", ty.Of(simple), builtin::AddressSpace::kUniform, Group(0_a), Binding(0_a));

    GeneratorImpl& gen = Build(Version(Version::Standard::kDesktop, 4, 4));
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 440

struct Simple {
  float member;
};

layout(binding = 0, std140) uniform Simple_ubo {
  float member;
} simple;

)");
}

}  // namespace
}  // namespace tint::writer::glsl
