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
#include "src/ast/variable.h"

namespace tint {
namespace {

using ScopeStackTest = testing::Test;

TEST_F(ScopeStackTest, Global) {
  ScopeStack<uint32_t> s;
  s.set_global("var", 5);

  uint32_t val = 0;
  EXPECT_TRUE(s.get("var", &val));
  EXPECT_EQ(val, 5u);
}

TEST_F(ScopeStackTest, Global_SetWithPointer) {
  ast::Variable v;
  v.set_name("my_var");

  ScopeStack<ast::Variable*> s;
  s.set_global("var", &v);

  ast::Variable* v2 = nullptr;
  EXPECT_TRUE(s.get("var", &v2));
  EXPECT_EQ(v2->name(), "my_var");
}

TEST_F(ScopeStackTest, Global_CanNotPop) {
  ScopeStack<uint32_t> s;
  s.set_global("var", 5);
  s.pop_scope();

  uint32_t val = 0;
  EXPECT_TRUE(s.get("var", &val));
  EXPECT_EQ(val, 5u);
}

TEST_F(ScopeStackTest, Scope) {
  ScopeStack<uint32_t> s;
  s.push_scope();
  s.set("var", 5);

  uint32_t val = 0;
  EXPECT_TRUE(s.get("var", &val));
  EXPECT_EQ(val, 5u);
}

TEST_F(ScopeStackTest, Get_MissingName) {
  ScopeStack<uint32_t> s;
  uint32_t ret = 0;
  EXPECT_FALSE(s.get("val", &ret));
  EXPECT_EQ(ret, 0u);
}

TEST_F(ScopeStackTest, Has) {
  ScopeStack<uint32_t> s;
  s.set_global("var2", 3);
  s.push_scope();
  s.set("var", 5);

  EXPECT_TRUE(s.has("var"));
  EXPECT_TRUE(s.has("var2"));
}

TEST_F(ScopeStackTest, ReturnsScopeBeforeGlobalFirst) {
  ScopeStack<uint32_t> s;
  s.set_global("var", 3);
  s.push_scope();
  s.set("var", 5);

  uint32_t ret;
  EXPECT_TRUE(s.get("var", &ret));
  EXPECT_EQ(ret, 5u);
}

}  // namespace
}  // namespace tint
