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
#include "src/tint/utils/text/string_stream.h"

#include "gmock/gmock.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::wgsl::writer {
namespace {

using WgslASTPrinterTest = TestHelper;

TEST_F(WgslASTPrinterTest, EmitVariable) {
    auto* v = GlobalVar("a", ty.f32(), core::AddressSpace::kPrivate);

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitVariable(out, v);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), R"(var<private> a : f32;)");
}

TEST_F(WgslASTPrinterTest, EmitVariable_AddressSpace) {
    auto* v = GlobalVar("a", ty.f32(), core::AddressSpace::kPrivate);

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitVariable(out, v);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), R"(var<private> a : f32;)");
}

TEST_F(WgslASTPrinterTest, EmitVariable_Access_Read) {
    auto* s = Structure("S", Vector{Member("a", ty.i32())});
    auto* v = GlobalVar("a", ty.Of(s), core::AddressSpace::kStorage, core::Access::kRead,
                        Binding(0_a), Group(0_a));

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitVariable(out, v);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), R"(@binding(0) @group(0) var<storage, read> a : S;)");
}

TEST_F(WgslASTPrinterTest, EmitVariable_Access_ReadWrite) {
    auto* s = Structure("S", Vector{Member("a", ty.i32())});
    auto* v = GlobalVar("a", ty.Of(s), core::AddressSpace::kStorage, core::Access::kReadWrite,
                        Binding(0_a), Group(0_a));

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitVariable(out, v);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), R"(@binding(0) @group(0) var<storage, read_write> a : S;)");
}

TEST_F(WgslASTPrinterTest, EmitVariable_Decorated) {
    auto* v = GlobalVar("a", ty.sampler(type::SamplerKind::kSampler), Group(1_a), Binding(2_a));

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitVariable(out, v);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), R"(@group(1) @binding(2) var a : sampler;)");
}

TEST_F(WgslASTPrinterTest, EmitVariable_Initializer) {
    auto* v = GlobalVar("a", ty.f32(), core::AddressSpace::kPrivate, Expr(1_f));

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitVariable(out, v);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), R"(var<private> a : f32 = 1.0f;)");
}

TEST_F(WgslASTPrinterTest, EmitVariable_Let_Explicit) {
    auto* v = Let("a", ty.f32(), Expr(1_f));
    WrapInFunction(v);

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitVariable(out, v);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), R"(let a : f32 = 1.0f;)");
}

TEST_F(WgslASTPrinterTest, EmitVariable_Let_Inferred) {
    auto* v = Let("a", Expr(1_f));
    WrapInFunction(v);

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitVariable(out, v);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), R"(let a = 1.0f;)");
}

TEST_F(WgslASTPrinterTest, EmitVariable_Const_Explicit) {
    auto* v = Const("a", ty.f32(), Expr(1_f));
    WrapInFunction(v);

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitVariable(out, v);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), R"(const a : f32 = 1.0f;)");
}

TEST_F(WgslASTPrinterTest, EmitVariable_Const_Inferred) {
    auto* v = Const("a", Expr(1_f));
    WrapInFunction(v);

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitVariable(out, v);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), R"(const a = 1.0f;)");
}

}  // namespace
}  // namespace tint::wgsl::writer
