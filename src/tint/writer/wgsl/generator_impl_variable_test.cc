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

#include "src/tint/utils/string_stream.h"
#include "src/tint/writer/wgsl/test_helper.h"

#include "gmock/gmock.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::wgsl {
namespace {

using WgslGeneratorImplTest = TestHelper;

TEST_F(WgslGeneratorImplTest, EmitVariable) {
    auto* v = GlobalVar("a", ty.f32(), builtin::AddressSpace::kPrivate);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    gen.EmitVariable(out, v);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), R"(var<private> a : f32;)");
}

TEST_F(WgslGeneratorImplTest, EmitVariable_AddressSpace) {
    auto* v = GlobalVar("a", ty.f32(), builtin::AddressSpace::kPrivate);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    gen.EmitVariable(out, v);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), R"(var<private> a : f32;)");
}

TEST_F(WgslGeneratorImplTest, EmitVariable_Access_Read) {
    auto* s = Structure("S", utils::Vector{Member("a", ty.i32())});
    auto* v = GlobalVar("a", ty.Of(s), builtin::AddressSpace::kStorage, builtin::Access::kRead,
                        Binding(0_a), Group(0_a));

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    gen.EmitVariable(out, v);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), R"(@binding(0) @group(0) var<storage, read> a : S;)");
}

TEST_F(WgslGeneratorImplTest, EmitVariable_Access_ReadWrite) {
    auto* s = Structure("S", utils::Vector{Member("a", ty.i32())});
    auto* v = GlobalVar("a", ty.Of(s), builtin::AddressSpace::kStorage, builtin::Access::kReadWrite,
                        Binding(0_a), Group(0_a));

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    gen.EmitVariable(out, v);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), R"(@binding(0) @group(0) var<storage, read_write> a : S;)");
}

TEST_F(WgslGeneratorImplTest, EmitVariable_Decorated) {
    auto* v = GlobalVar("a", ty.sampler(type::SamplerKind::kSampler), Group(1_a), Binding(2_a));

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    gen.EmitVariable(out, v);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), R"(@group(1) @binding(2) var a : sampler;)");
}

TEST_F(WgslGeneratorImplTest, EmitVariable_Initializer) {
    auto* v = GlobalVar("a", ty.f32(), builtin::AddressSpace::kPrivate, Expr(1_f));

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    gen.EmitVariable(out, v);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), R"(var<private> a : f32 = 1.0f;)");
}

TEST_F(WgslGeneratorImplTest, EmitVariable_Let_Explicit) {
    auto* v = Let("a", ty.f32(), Expr(1_f));
    WrapInFunction(v);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    gen.EmitVariable(out, v);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), R"(let a : f32 = 1.0f;)");
}

TEST_F(WgslGeneratorImplTest, EmitVariable_Let_Inferred) {
    auto* v = Let("a", Expr(1_f));
    WrapInFunction(v);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    gen.EmitVariable(out, v);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), R"(let a = 1.0f;)");
}

TEST_F(WgslGeneratorImplTest, EmitVariable_Const_Explicit) {
    auto* v = Const("a", ty.f32(), Expr(1_f));
    WrapInFunction(v);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    gen.EmitVariable(out, v);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), R"(const a : f32 = 1.0f;)");
}

TEST_F(WgslGeneratorImplTest, EmitVariable_Const_Inferred) {
    auto* v = Const("a", Expr(1_f));
    WrapInFunction(v);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    gen.EmitVariable(out, v);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), R"(const a = 1.0f;)");
}

}  // namespace
}  // namespace tint::writer::wgsl
