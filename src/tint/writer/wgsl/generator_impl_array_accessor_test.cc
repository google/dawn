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

TEST_F(WgslGeneratorImplTest, IndexAccessor) {
    GlobalVar("ary", ty.array<i32, 10>(), builtin::AddressSpace::kPrivate);
    auto* expr = IndexAccessor("ary", 5_i);
    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    gen.EmitExpression(out, expr);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "ary[5i]");
}

TEST_F(WgslGeneratorImplTest, IndexAccessor_OfDref) {
    GlobalVar("ary", ty.array<i32, 10>(), builtin::AddressSpace::kPrivate);

    auto* p = Let("p", AddressOf("ary"));
    auto* expr = IndexAccessor(Deref("p"), 5_i);
    WrapInFunction(p, expr);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    gen.EmitExpression(out, expr);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "(*(p))[5i]");
}

}  // namespace
}  // namespace tint::writer::wgsl
