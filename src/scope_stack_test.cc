// Copyright 2020 The Tint Authors.  //
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

#include "src/scope_stack.h"

#include "gtest/gtest.h"
#include "src/program_builder.h"

namespace tint {
namespace {

class ScopeStackTest : public ProgramBuilder, public testing::Test {};

TEST_F(ScopeStackTest, Global) {
  ScopeStack<uint32_t> s;
  Symbol sym(1, ID());
  s.set_global(sym, 5);

  uint32_t val = 0;
  EXPECT_TRUE(s.get(sym, &val));
  EXPECT_EQ(val, 5u);
}

TEST_F(ScopeStackTest, Global_SetWithPointer) {
  auto* v = Var("my_var", ty.f32(), ast::StorageClass::kNone);
  ScopeStack<ast::Variable*> s;
  s.set_global(v->symbol(), v);

  ast::Variable* v2 = nullptr;
  EXPECT_TRUE(s.get(v->symbol(), &v2));
  EXPECT_EQ(v2->symbol(), v->symbol());
}

TEST_F(ScopeStackTest, Global_CanNotPop) {
  ScopeStack<uint32_t> s;
  Symbol sym(1, ID());
  s.set_global(sym, 5);
  s.pop_scope();

  uint32_t val = 0;
  EXPECT_TRUE(s.get(sym, &val));
  EXPECT_EQ(val, 5u);
}

TEST_F(ScopeStackTest, Scope) {
  ScopeStack<uint32_t> s;
  Symbol sym(1, ID());
  s.push_scope();
  s.set(sym, 5);

  uint32_t val = 0;
  EXPECT_TRUE(s.get(sym, &val));
  EXPECT_EQ(val, 5u);
}

TEST_F(ScopeStackTest, Get_MissingSymbol) {
  ScopeStack<uint32_t> s;
  Symbol sym(1, ID());
  uint32_t ret = 0;
  EXPECT_FALSE(s.get(sym, &ret));
  EXPECT_EQ(ret, 0u);
}

TEST_F(ScopeStackTest, Has) {
  ScopeStack<uint32_t> s;
  Symbol sym(1, ID());
  Symbol sym2(2, ID());
  s.set_global(sym2, 3);
  s.push_scope();
  s.set(sym, 5);

  EXPECT_TRUE(s.has(sym));
  EXPECT_TRUE(s.has(sym2));
}

TEST_F(ScopeStackTest, ReturnsScopeBeforeGlobalFirst) {
  ScopeStack<uint32_t> s;
  Symbol sym(1, ID());
  s.set_global(sym, 3);
  s.push_scope();
  s.set(sym, 5);

  uint32_t ret;
  EXPECT_TRUE(s.get(sym, &ret));
  EXPECT_EQ(ret, 5u);
}

}  // namespace
}  // namespace tint
