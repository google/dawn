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

#include "src/tint/lang/wgsl/writer/ast_printer/helper_test.h"

#include "gmock/gmock.h"

namespace tint::wgsl::writer {
namespace {

using WgslASTPrinterTest = TestHelper;

TEST_F(WgslASTPrinterTest, EmitAlias_F32) {
    auto* alias = Alias("a", ty.f32());

    ASTPrinter& gen = Build();
    gen.EmitTypeDecl(alias);

    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(alias a = f32;
)");
}

TEST_F(WgslASTPrinterTest, EmitTypeDecl_Struct) {
    auto* s = Structure("A", Vector{
                                 Member("a", ty.f32()),
                                 Member("b", ty.i32()),
                             });

    auto* alias = Alias("B", ty.Of(s));

    ASTPrinter& gen = Build();
    gen.EmitTypeDecl(s);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());

    gen.EmitTypeDecl(alias);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(struct A {
  a : f32,
  b : i32,
}
alias B = A;
)");
}

TEST_F(WgslASTPrinterTest, EmitAlias_ToStruct) {
    auto* s = Structure("A", Vector{
                                 Member("a", ty.f32()),
                                 Member("b", ty.i32()),
                             });

    auto* alias = Alias("B", ty.Of(s));

    ASTPrinter& gen = Build();

    gen.EmitTypeDecl(alias);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(alias B = A;
)");
}

}  // namespace
}  // namespace tint::wgsl::writer
