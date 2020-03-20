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

#include "src/reader/spirv/namer.h"

#include <cstdint>
#include <sstream>
#include <string>

#include "gtest/gtest.h"
#include "src/reader/spirv/fail_stream.h"

namespace tint {
namespace reader {
namespace spirv {
namespace {

class SpvNamerTest : public testing::Test {
 public:
  SpvNamerTest() : fail_stream_(&success_, &errors_) {}

  /// @returns the accumulated diagnostic strings
  std::string error() { return errors_.str(); }

 protected:
  std::stringstream errors_;
  bool success_ = true;
  FailStream fail_stream_;
};

TEST_F(SpvNamerTest, NoFailureToStart) {
  Namer namer(fail_stream_);
  EXPECT_TRUE(success_);
  EXPECT_TRUE(error().empty());
}

TEST_F(SpvNamerTest, FailLogsError) {
  Namer namer(fail_stream_);
  const bool converted_result = namer.Fail() << "st. johns wood";
  EXPECT_FALSE(converted_result);
  EXPECT_EQ(error(), "st. johns wood");
  EXPECT_FALSE(success_);
}

TEST_F(SpvNamerTest, NoNameRecorded) {
  Namer namer(fail_stream_);

  EXPECT_FALSE(namer.HasName(12));
  EXPECT_TRUE(success_);
  EXPECT_TRUE(error().empty());
}

TEST_F(SpvNamerTest, SaveNameOnce) {
  Namer namer(fail_stream_);

  const uint32_t id = 9;
  EXPECT_FALSE(namer.HasName(id));
  const bool save_result = namer.SaveName(id, "abbey road");
  EXPECT_TRUE(save_result);
  EXPECT_TRUE(namer.HasName(id));
  EXPECT_EQ(namer.GetName(id), "abbey road");
  EXPECT_TRUE(success_);
  EXPECT_TRUE(error().empty());
}

TEST_F(SpvNamerTest, SaveNameTwoIds) {
  Namer namer(fail_stream_);

  EXPECT_FALSE(namer.HasName(8));
  EXPECT_FALSE(namer.HasName(9));
  EXPECT_TRUE(namer.SaveName(8, "abbey road"));
  EXPECT_TRUE(namer.SaveName(9, "rubber soul"));
  EXPECT_TRUE(namer.HasName(8));
  EXPECT_TRUE(namer.HasName(9));
  EXPECT_EQ(namer.GetName(9), "rubber soul");
  EXPECT_EQ(namer.GetName(8), "abbey road");
  EXPECT_TRUE(success_);
  EXPECT_TRUE(error().empty());
}

TEST_F(SpvNamerTest, SaveNameFailsDueToIdReuse) {
  Namer namer(fail_stream_);

  const uint32_t id = 9;
  EXPECT_TRUE(namer.SaveName(id, "abbey road"));
  EXPECT_FALSE(namer.SaveName(id, "rubber soul"));
  EXPECT_TRUE(namer.HasName(id));
  EXPECT_EQ(namer.GetName(id), "abbey road");
  EXPECT_FALSE(success_);
  EXPECT_FALSE(error().empty());
}

}  // namespace
}  // namespace spirv
}  // namespace reader
}  // namespace tint
