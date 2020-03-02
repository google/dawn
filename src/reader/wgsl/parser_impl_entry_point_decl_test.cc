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
#include "src/ast/variable.h"
#include "src/reader/wgsl/parser_impl.h"

namespace tint {
namespace reader {
namespace wgsl {

using ParserImplTest = testing::Test;

TEST_F(ParserImplTest, EntryPoint_Parses) {
  ParserImpl p{"entry_point fragment = main"};
  auto e = p.entry_point_decl();
  ASSERT_NE(e, nullptr);
  ASSERT_FALSE(p.has_error());
  EXPECT_EQ(e->stage(), ast::PipelineStage::kFragment);
  EXPECT_EQ(e->name(), "main");
  EXPECT_EQ(e->function_name(), "main");
}

TEST_F(ParserImplTest, EntryPoint_ParsesWithStringName) {
  ParserImpl p{R"(entry_point vertex as "main" = vtx_main)"};
  auto e = p.entry_point_decl();
  ASSERT_NE(e, nullptr);
  ASSERT_FALSE(p.has_error());
  EXPECT_EQ(e->stage(), ast::PipelineStage::kVertex);
  EXPECT_EQ(e->name(), "main");
  EXPECT_EQ(e->function_name(), "vtx_main");
}

TEST_F(ParserImplTest, EntryPoint_ParsesWithIdentName) {
  ParserImpl p{R"(entry_point vertex as main = vtx_main)"};
  auto e = p.entry_point_decl();
  ASSERT_NE(e, nullptr);
  ASSERT_FALSE(p.has_error());
  EXPECT_EQ(e->stage(), ast::PipelineStage::kVertex);
  EXPECT_EQ(e->name(), "main");
  EXPECT_EQ(e->function_name(), "vtx_main");
}

TEST_F(ParserImplTest, EntryPoint_MissingFnName) {
  ParserImpl p{R"(entry_point vertex as main =)"};
  auto e = p.entry_point_decl();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:29: invalid function name for entry point");
}

TEST_F(ParserImplTest, EntryPoint_InvalidFnName) {
  ParserImpl p{R"(entry_point vertex as main = 123)"};
  auto e = p.entry_point_decl();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:30: invalid function name for entry point");
}

TEST_F(ParserImplTest, EntryPoint_MissingEqual) {
  ParserImpl p{R"(entry_point vertex as main vtx_main)"};
  auto e = p.entry_point_decl();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:28: missing = for entry point");
}

TEST_F(ParserImplTest, EntryPoint_MissingName) {
  ParserImpl p{R"(entry_point vertex as = vtx_main)"};
  auto e = p.entry_point_decl();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:23: invalid name for entry point");
}

TEST_F(ParserImplTest, EntryPoint_InvalidName) {
  ParserImpl p{R"(entry_point vertex as 123 = vtx_main)"};
  auto e = p.entry_point_decl();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:23: invalid name for entry point");
}

TEST_F(ParserImplTest, EntryPoint_MissingStageWithIdent) {
  ParserImpl p{R"(entry_point as 123 = vtx_main)"};
  auto e = p.entry_point_decl();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:13: missing pipeline stage for entry point");
}

TEST_F(ParserImplTest, EntryPoint_MissingStage) {
  ParserImpl p{R"(entry_point = vtx_main)"};
  auto e = p.entry_point_decl();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:13: missing pipeline stage for entry point");
}

TEST_F(ParserImplTest, EntryPoint_InvalidStage) {
  ParserImpl p{R"(entry_point invalid = vtx_main)"};
  auto e = p.entry_point_decl();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:13: missing pipeline stage for entry point");
}

}  // namespace wgsl
}  // namespace reader
}  // namespace tint
