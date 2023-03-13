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

#include "src/tint/sem/value_expression.h"

#include "src/tint/sem/test_helper.h"

#include "src/tint/sem/materialize.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::sem {
namespace {

class MockConstant : public constant::Value {
  public:
    explicit MockConstant(const type::Type* ty) : type(ty) {}
    ~MockConstant() override {}
    const type::Type* Type() const override { return type; }
    const constant::Value* Index(size_t) const override { return {}; }
    size_t NumElements() const override { return 0; }
    bool AllZero() const override { return {}; }
    bool AnyZero() const override { return {}; }
    size_t Hash() const override { return 0; }
    MockConstant* Clone(constant::CloneContext&) const override { return nullptr; }

  protected:
    std::variant<std::monostate, AInt, AFloat> InternalValue() const override { return {}; }

  private:
    const type::Type* type;
};

using ValueExpressionTest = TestHelper;

TEST_F(ValueExpressionTest, UnwrapMaterialize) {
    MockConstant c(create<type::I32>());
    auto* a = create<ValueExpression>(/* declaration */ nullptr, create<type::I32>(),
                                      sem::EvaluationStage::kRuntime, /* statement */ nullptr,
                                      /* constant_value */ nullptr,
                                      /* has_side_effects */ false, /* root_ident */ nullptr);
    auto* b = create<Materialize>(a, /* statement */ nullptr, c.Type(), &c);

    EXPECT_EQ(a, a->UnwrapMaterialize());
    EXPECT_EQ(a, b->UnwrapMaterialize());
}

}  // namespace
}  // namespace tint::sem
