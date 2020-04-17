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

#include "src/ast/import.h"

#include <sstream>

#include "gtest/gtest.h"

namespace tint {
namespace ast {
namespace {

using ImportTest = testing::Test;

TEST_F(ImportTest, Creation) {
  Import i("GLSL.std.430", "std::glsl");

  EXPECT_EQ(i.path(), "GLSL.std.430");
  EXPECT_EQ(i.name(), "std::glsl");
  EXPECT_EQ(i.line(), 0u);
  EXPECT_EQ(i.column(), 0u);
}

TEST_F(ImportTest, CreationWithSource) {
  Source s{27, 4};
  Import i(s, "GLSL.std.430", "std::glsl");

  EXPECT_EQ(i.path(), "GLSL.std.430");
  EXPECT_EQ(i.name(), "std::glsl");
  EXPECT_EQ(i.line(), 27u);
  EXPECT_EQ(i.column(), 4u);
}

TEST_F(ImportTest, CreationEmpty) {
  Source s{27, 4};
  Import i;
  i.set_source(s);
  i.set_path("GLSL.std.430");
  i.set_name("std::glsl");

  EXPECT_EQ(i.path(), "GLSL.std.430");
  EXPECT_EQ(i.name(), "std::glsl");
  EXPECT_EQ(i.line(), 27u);
  EXPECT_EQ(i.column(), 4u);
}

TEST_F(ImportTest, to_str) {
  Import i{"GLSL.std.430", "std::glsl"};
  std::ostringstream out;
  i.to_str(out, 2);
  EXPECT_EQ(out.str(), "  Import{\"GLSL.std.430\" as std::glsl}\n");
}

TEST_F(ImportTest, IsValid) {
  Import i{"GLSL.std.430", "std::glsl"};
  EXPECT_TRUE(i.IsValid());
}

TEST_F(ImportTest, IsValid_MissingPath) {
  Import i{"", "std::glsl"};
  EXPECT_FALSE(i.IsValid());
}

TEST_F(ImportTest, IsValid_MissingName) {
  Import i{"GLSL.std.430", ""};
  EXPECT_FALSE(i.IsValid());
}

TEST_F(ImportTest, IsValid_MissingBoth) {
  Import i;
  EXPECT_FALSE(i.IsValid());
}

TEST_F(ImportTest, IsValid_InvalidEndingCharacter) {
  Import i{"GLSL.std.430", "std::glsl::"};
  EXPECT_FALSE(i.IsValid());
}

}  // namespace
}  // namespace ast
}  // namespace tint
