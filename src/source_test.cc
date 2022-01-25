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

#include "src/source.h"

#include <memory>
#include <utility>

#include "gtest/gtest.h"

namespace tint {
namespace {

static constexpr const char* kSource = R"(line one
line two
line three)";

using SourceFileContentTest = testing::Test;

TEST_F(SourceFileContentTest, Ctor) {
  Source::FileContent fc(kSource);
  EXPECT_EQ(fc.data, kSource);
  EXPECT_EQ(fc.data_view, kSource);
  ASSERT_EQ(fc.lines.size(), 3u);
  EXPECT_EQ(fc.lines[0], "line one");
  EXPECT_EQ(fc.lines[1], "line two");
  EXPECT_EQ(fc.lines[2], "line three");
}

TEST_F(SourceFileContentTest, CopyCtor) {
  auto src = std::make_unique<Source::FileContent>(kSource);
  Source::FileContent fc{*src};
  src.reset();
  EXPECT_EQ(fc.data, kSource);
  EXPECT_EQ(fc.data_view, kSource);
  ASSERT_EQ(fc.lines.size(), 3u);
  EXPECT_EQ(fc.lines[0], "line one");
  EXPECT_EQ(fc.lines[1], "line two");
  EXPECT_EQ(fc.lines[2], "line three");
}

TEST_F(SourceFileContentTest, MoveCtor) {
  auto src = std::make_unique<Source::FileContent>(kSource);
  Source::FileContent fc{std::move(*src)};
  src.reset();
  EXPECT_EQ(fc.data, kSource);
  EXPECT_EQ(fc.data_view, kSource);
  ASSERT_EQ(fc.lines.size(), 3u);
  EXPECT_EQ(fc.lines[0], "line one");
  EXPECT_EQ(fc.lines[1], "line two");
  EXPECT_EQ(fc.lines[2], "line three");
}

}  // namespace
}  // namespace tint
