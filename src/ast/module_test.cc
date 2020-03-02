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

#include "src/ast/module.h"

#include <utility>

#include "gtest/gtest.h"

namespace tint {
namespace ast {

using ModuleTest = testing::Test;

TEST_F(ModuleTest, Creation) {
  Module m;

  EXPECT_EQ(m.imports().size(), 0);
}

TEST_F(ModuleTest, Imports) {
  Module m;

  m.AddImport(std::make_unique<Import>("GLSL.std.430", "std::glsl"));
  m.AddImport(std::make_unique<Import>("OpenCL.debug.100", "std::debug"));

  EXPECT_EQ(2, m.imports().size());
  EXPECT_EQ("std::glsl", m.imports()[0]->name());
}

TEST_F(ModuleTest, LookupImport) {
  Module m;

  auto i = std::make_unique<Import>("GLSL.std.430", "std::glsl");
  m.AddImport(std::move(i));
  m.AddImport(std::make_unique<Import>("OpenCL.debug.100", "std::debug"));

  auto import = m.FindImportByName("std::glsl");
  ASSERT_NE(nullptr, import);
  EXPECT_EQ(import->path(), "GLSL.std.430");
  EXPECT_EQ(import->name(), "std::glsl");
}

TEST_F(ModuleTest, LookupImportMissing) {
  Module m;
  EXPECT_EQ(nullptr, m.FindImportByName("Missing"));
}

TEST_F(ModuleTest, IsValid_Empty) {
  Module m;
  EXPECT_TRUE(m.IsValid());
}

TEST_F(ModuleTest, IsValid_InvalidImport) {
  Module m;
  m.AddImport(std::make_unique<Import>());
  EXPECT_FALSE(m.IsValid());
}

TEST_F(ModuleTest, IsValid_ValidImport) {
  Module m;
  m.AddImport(std::make_unique<Import>("GLSL.std.430", "std::glsl"));
  EXPECT_TRUE(m.IsValid());
}

}  // namespace ast
}  // namespace tint
