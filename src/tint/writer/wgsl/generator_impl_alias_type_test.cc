// Copyright 2020 The Tint Authors.
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

#include "src/tint/writer/wgsl/test_helper.h"

#include "gmock/gmock.h"

namespace tint::writer::wgsl {
namespace {

using WgslGeneratorImplTest = TestHelper;

TEST_F(WgslGeneratorImplTest, EmitAlias_F32) {
    auto* alias = Alias("a", ty.f32());

    GeneratorImpl& gen = Build();
    gen.EmitTypeDecl(alias);

    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(alias a = f32;
)");
}

TEST_F(WgslGeneratorImplTest, EmitTypeDecl_Struct) {
    auto* s = Structure("A", utils::Vector{
                                 Member("a", ty.f32()),
                                 Member("b", ty.i32()),
                             });

    auto* alias = Alias("B", ty.Of(s));

    GeneratorImpl& gen = Build();
    gen.EmitTypeDecl(s);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());

    gen.EmitTypeDecl(alias);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(struct A {
  a : f32,
  b : i32,
}
alias B = A;
)");
}

TEST_F(WgslGeneratorImplTest, EmitAlias_ToStruct) {
    auto* s = Structure("A", utils::Vector{
                                 Member("a", ty.f32()),
                                 Member("b", ty.i32()),
                             });

    auto* alias = Alias("B", ty.Of(s));

    GeneratorImpl& gen = Build();

    gen.EmitTypeDecl(alias);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(alias B = A;
)");
}

}  // namespace
}  // namespace tint::writer::wgsl
