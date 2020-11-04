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

#include "gtest/gtest.h"
#include "src/ast/function.h"
#include "src/ast/type/type.h"
#include "src/ast/workgroup_decoration.h"
#include "src/reader/wgsl/parser_impl.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, FunctionDecl) {
  auto* p = parser("fn main(a : i32, b : f32) -> void { return; }");
  auto f = p->function_decl();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(f, nullptr);

  EXPECT_EQ(f->name(), "main");
  ASSERT_NE(f->return_type(), nullptr);
  EXPECT_TRUE(f->return_type()->IsVoid());

  ASSERT_EQ(f->params().size(), 2u);
  EXPECT_EQ(f->params()[0]->name(), "a");
  EXPECT_EQ(f->params()[1]->name(), "b");

  ASSERT_NE(f->return_type(), nullptr);
  EXPECT_TRUE(f->return_type()->IsVoid());

  auto* body = f->body();
  ASSERT_EQ(body->size(), 1u);
  EXPECT_TRUE(body->get(0)->IsReturn());
}

TEST_F(ParserImplTest, FunctionDecl_DecorationList) {
  auto* p = parser("[[workgroup_size(2, 3, 4)]] fn main() -> void { return; }");
  auto f = p->function_decl();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(f, nullptr);

  EXPECT_EQ(f->name(), "main");
  ASSERT_NE(f->return_type(), nullptr);
  EXPECT_TRUE(f->return_type()->IsVoid());
  ASSERT_EQ(f->params().size(), 0u);
  ASSERT_NE(f->return_type(), nullptr);
  EXPECT_TRUE(f->return_type()->IsVoid());

  auto& decos = f->decorations();
  ASSERT_EQ(decos.size(), 1u);
  ASSERT_TRUE(decos[0]->IsWorkgroup());

  uint32_t x = 0;
  uint32_t y = 0;
  uint32_t z = 0;
  std::tie(x, y, z) = decos[0]->AsWorkgroup()->values();
  EXPECT_EQ(x, 2u);
  EXPECT_EQ(y, 3u);
  EXPECT_EQ(z, 4u);

  auto* body = f->body();
  ASSERT_EQ(body->size(), 1u);
  EXPECT_TRUE(body->get(0)->IsReturn());
}

TEST_F(ParserImplTest, FunctionDecl_DecorationList_MultipleEntries) {
  auto* p = parser(R"(
[[workgroup_size(2, 3, 4), workgroup_size(5, 6, 7)]]
fn main() -> void { return; })");
  auto f = p->function_decl();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(f, nullptr);

  EXPECT_EQ(f->name(), "main");
  ASSERT_NE(f->return_type(), nullptr);
  EXPECT_TRUE(f->return_type()->IsVoid());
  ASSERT_EQ(f->params().size(), 0u);
  ASSERT_NE(f->return_type(), nullptr);
  EXPECT_TRUE(f->return_type()->IsVoid());

  auto& decos = f->decorations();
  ASSERT_EQ(decos.size(), 2u);

  uint32_t x = 0;
  uint32_t y = 0;
  uint32_t z = 0;
  ASSERT_TRUE(decos[0]->IsWorkgroup());
  std::tie(x, y, z) = decos[0]->AsWorkgroup()->values();
  EXPECT_EQ(x, 2u);
  EXPECT_EQ(y, 3u);
  EXPECT_EQ(z, 4u);

  ASSERT_TRUE(decos[1]->IsWorkgroup());
  std::tie(x, y, z) = decos[1]->AsWorkgroup()->values();
  EXPECT_EQ(x, 5u);
  EXPECT_EQ(y, 6u);
  EXPECT_EQ(z, 7u);

  auto* body = f->body();
  ASSERT_EQ(body->size(), 1u);
  EXPECT_TRUE(body->get(0)->IsReturn());
}

TEST_F(ParserImplTest, FunctionDecl_DecorationList_MultipleLists) {
  auto* p = parser(R"(
[[workgroup_size(2, 3, 4)]]
[[workgroup_size(5, 6, 7)]]
fn main() -> void { return; })");
  auto f = p->function_decl();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(f, nullptr);

  EXPECT_EQ(f->name(), "main");
  ASSERT_NE(f->return_type(), nullptr);
  EXPECT_TRUE(f->return_type()->IsVoid());
  ASSERT_EQ(f->params().size(), 0u);
  ASSERT_NE(f->return_type(), nullptr);
  EXPECT_TRUE(f->return_type()->IsVoid());

  auto& decos = f->decorations();
  ASSERT_EQ(decos.size(), 2u);

  uint32_t x = 0;
  uint32_t y = 0;
  uint32_t z = 0;
  ASSERT_TRUE(decos[0]->IsWorkgroup());
  std::tie(x, y, z) = decos[0]->AsWorkgroup()->values();
  EXPECT_EQ(x, 2u);
  EXPECT_EQ(y, 3u);
  EXPECT_EQ(z, 4u);

  ASSERT_TRUE(decos[1]->IsWorkgroup());
  std::tie(x, y, z) = decos[1]->AsWorkgroup()->values();
  EXPECT_EQ(x, 5u);
  EXPECT_EQ(y, 6u);
  EXPECT_EQ(z, 7u);

  auto* body = f->body();
  ASSERT_EQ(body->size(), 1u);
  EXPECT_TRUE(body->get(0)->IsReturn());
}

TEST_F(ParserImplTest, FunctionDecl_InvalidHeader) {
  auto* p = parser("fn main() -> { }");
  auto f = p->function_decl();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(f, nullptr);
  EXPECT_EQ(p->error(), "1:14: unable to determine function return type");
}

TEST_F(ParserImplTest, FunctionDecl_InvalidBody) {
  auto* p = parser("fn main() -> void { return }");
  auto f = p->function_decl();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(f, nullptr);
  EXPECT_EQ(p->error(), "1:28: expected ';' for return statement");
}

TEST_F(ParserImplTest, FunctionDecl_MissingLeftBrace) {
  auto* p = parser("fn main() -> void return; }");
  auto f = p->function_decl();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(f, nullptr);
  EXPECT_EQ(p->error(), "1:19: expected '{'");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
