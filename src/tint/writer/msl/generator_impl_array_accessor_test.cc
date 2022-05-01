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

#include "src/tint/writer/msl/test_helper.h"

namespace tint::writer::msl {
namespace {

using MslGeneratorImplTest = TestHelper;

TEST_F(MslGeneratorImplTest, IndexAccessor) {
    auto* ary = Var("ary", ty.array<i32, 10>());
    auto* expr = IndexAccessor("ary", 5);
    WrapInFunction(ary, expr);

    GeneratorImpl& gen = Build();

    std::stringstream out;
    ASSERT_TRUE(gen.EmitExpression(out, expr)) << gen.error();
    EXPECT_EQ(out.str(), "ary[5]");
}

TEST_F(MslGeneratorImplTest, IndexAccessor_OfDref) {
    Global("ary", ty.array<i32, 10>(), ast::StorageClass::kPrivate);

    auto* p = Let("p", nullptr, AddressOf("ary"));
    auto* expr = IndexAccessor(Deref("p"), 5);
    WrapInFunction(p, expr);

    GeneratorImpl& gen = Build();

    std::stringstream out;
    ASSERT_TRUE(gen.EmitExpression(out, expr)) << gen.error();
    EXPECT_EQ(out.str(), "(*(p))[5]");
}

}  // namespace
}  // namespace tint::writer::msl
