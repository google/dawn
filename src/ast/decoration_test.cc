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

#include "src/ast/decoration.h"

#include <sstream>
#include <utility>

#include "src/ast/array_decoration.h"
#include "src/ast/constant_id_decoration.h"
#include "src/ast/test_helper.h"

namespace tint {
namespace ast {
namespace {

using DecorationTest = TestHelper;

TEST_F(DecorationTest, AsCorrectType) {
  auto* decoration = new ConstantIdDecoration(1, Source{});
  auto upcast = std::unique_ptr<Decoration>(decoration);
  auto downcast = As<VariableDecoration>(std::move(upcast));
  EXPECT_EQ(decoration, downcast.get());
}

TEST_F(DecorationTest, AsIncorrectType) {
  auto* decoration = new ConstantIdDecoration(1, Source{});
  auto upcast = std::unique_ptr<Decoration>(decoration);
  auto downcast = As<ArrayDecoration>(std::move(upcast));
  EXPECT_EQ(nullptr, downcast.get());
}

TEST_F(DecorationTest, Is) {
  auto decoration = create<ConstantIdDecoration>(1, Source{});
  EXPECT_TRUE(decoration->Is<VariableDecoration>());
  EXPECT_FALSE(decoration->Is<ArrayDecoration>());
}

}  // namespace
}  // namespace ast
}  // namespace tint
