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

#include "gmock/gmock.h"
#include "src/reader/spirv/fail_stream.h"

namespace tint {
namespace reader {
namespace spirv {
namespace {

using ::testing::Eq;

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

TEST_F(SpvNamerTest, SanitizeEmpty) {
  EXPECT_THAT(Namer::Sanitize(""), Eq("empty"));
}

TEST_F(SpvNamerTest, SanitizeLeadingUnderscore) {
  EXPECT_THAT(Namer::Sanitize("_"), Eq("x_"));
}

TEST_F(SpvNamerTest, SanitizeLeadingDigit) {
  EXPECT_THAT(Namer::Sanitize("7zip"), Eq("x7zip"));
}

TEST_F(SpvNamerTest, SanitizeOkChars) {
  EXPECT_THAT(Namer::Sanitize("_abcdef12345"), Eq("x_abcdef12345"));
}

TEST_F(SpvNamerTest, SanitizeNonIdentifierChars) {
  EXPECT_THAT(Namer::Sanitize("a:1.2'f\n"), "a_1_2_f_");
}

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

TEST_F(SpvNamerTest, FindUnusedDerivedName_NoRecordedName) {
  Namer namer(fail_stream_);
  EXPECT_THAT(namer.FindUnusedDerivedName("eleanor"), Eq("eleanor"));
}

TEST_F(SpvNamerTest, FindUnusedDerivedName_HasRecordedName) {
  Namer namer(fail_stream_);
  namer.SaveName(12, "rigby");
  EXPECT_THAT(namer.FindUnusedDerivedName("rigby"), Eq("rigby_1"));
}

TEST_F(SpvNamerTest, FindUnusedDerivedName_HasMultipleConflicts) {
  Namer namer(fail_stream_);
  namer.SaveName(12, "rigby");
  namer.SaveName(13, "rigby_1");
  namer.SaveName(14, "rigby_3");
  // It picks the first non-conflicting suffix.
  EXPECT_THAT(namer.FindUnusedDerivedName("rigby"), Eq("rigby_2"));
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

TEST_F(SpvNamerTest, SuggestSanitizedName_TakeSuggestionWhenNoConflict) {
  Namer namer(fail_stream_);

  EXPECT_TRUE(namer.SuggestSanitizedName(1, "father"));
  EXPECT_THAT(namer.GetName(1), Eq("father"));
}

TEST_F(SpvNamerTest,
       SuggestSanitizedName_RejectSuggestionWhenConflictOnSameId) {
  Namer namer(fail_stream_);

  namer.SaveName(1, "lennon");
  EXPECT_FALSE(namer.SuggestSanitizedName(1, "mccartney"));
  EXPECT_THAT(namer.GetName(1), Eq("lennon"));
}

TEST_F(SpvNamerTest, SuggestSanitizedName_SanitizeSuggestion) {
  Namer namer(fail_stream_);

  EXPECT_TRUE(namer.SuggestSanitizedName(9, "m:kenzie"));
  EXPECT_THAT(namer.GetName(9), Eq("m_kenzie"));
}

TEST_F(SpvNamerTest,
       SuggestSanitizedName_GenerateNewNameWhenConflictOnDifferentId) {
  Namer namer(fail_stream_);

  namer.SaveName(7, "rice");
  EXPECT_TRUE(namer.SuggestSanitizedName(9, "rice"));
  EXPECT_THAT(namer.GetName(9), Eq("rice_1"));
}

TEST_F(SpvNamerTest, GetMemberName_EmptyStringForUnvisitedStruct) {
  Namer namer(fail_stream_);
  EXPECT_THAT(namer.GetMemberName(1, 2), Eq(""));
}

TEST_F(SpvNamerTest, GetMemberName_EmptyStringForUnvisitedMember) {
  Namer namer(fail_stream_);
  namer.SuggestSanitizedMemberName(1, 2, "mother");
  EXPECT_THAT(namer.GetMemberName(1, 0), Eq(""));
}

TEST_F(SpvNamerTest, SuggestSanitizedMemberName_TakeSuggestionWhenNoConflict) {
  Namer namer(fail_stream_);
  EXPECT_TRUE(namer.SuggestSanitizedMemberName(1, 2, "mother"));
  EXPECT_THAT(namer.GetMemberName(1, 2), Eq("mother"));
}

TEST_F(SpvNamerTest, SuggestSanitizedMemberName_TakeSanitizedSuggestion) {
  Namer namer(fail_stream_);
  EXPECT_TRUE(namer.SuggestSanitizedMemberName(1, 2, "m:t%er"));
  EXPECT_THAT(namer.GetMemberName(1, 2), Eq("m_t_er"));
}

TEST_F(
    SpvNamerTest,
    SuggestSanitizedMemberName_TakeSuggestionWhenNoConflictAfterSuggestionForLowerMember) {
  Namer namer(fail_stream_);
  EXPECT_TRUE(namer.SuggestSanitizedMemberName(1, 7, "mother"));
  EXPECT_THAT(namer.GetMemberName(1, 2), Eq(""));
  EXPECT_TRUE(namer.SuggestSanitizedMemberName(1, 2, "mary"));
  EXPECT_THAT(namer.GetMemberName(1, 2), Eq("mary"));
}

TEST_F(SpvNamerTest,
       SuggestSanitizedMemberName_RejectSuggestionIfConflictOnMember) {
  Namer namer(fail_stream_);
  EXPECT_TRUE(namer.SuggestSanitizedMemberName(1, 2, "mother"));
  EXPECT_FALSE(namer.SuggestSanitizedMemberName(1, 2, "mary"));
  EXPECT_THAT(namer.GetMemberName(1, 2), Eq("mother"));
}

TEST_F(SpvNamerTest,
       ResolveMemberNamesForStruct_GeneratesRegularNamesOnItsOwn) {
  Namer namer(fail_stream_);
  namer.ResolveMemberNamesForStruct(2, 4);
  EXPECT_THAT(namer.GetMemberName(2, 0), Eq("field0"));
  EXPECT_THAT(namer.GetMemberName(2, 1), Eq("field1"));
  EXPECT_THAT(namer.GetMemberName(2, 2), Eq("field2"));
  EXPECT_THAT(namer.GetMemberName(2, 3), Eq("field3"));
}

TEST_F(SpvNamerTest,
       ResolveMemberNamesForStruct_ResolvesConflictBetweenSuggestedNames) {
  Namer namer(fail_stream_);
  namer.SuggestSanitizedMemberName(2, 0, "apple");
  namer.SuggestSanitizedMemberName(2, 1, "apple");
  namer.ResolveMemberNamesForStruct(2, 2);
  EXPECT_THAT(namer.GetMemberName(2, 0), Eq("apple"));
  EXPECT_THAT(namer.GetMemberName(2, 1), Eq("apple_1"));
}

TEST_F(SpvNamerTest, ResolveMemberNamesForStruct_FillsUnsuggestedGaps) {
  Namer namer(fail_stream_);
  namer.SuggestSanitizedMemberName(2, 1, "apple");
  namer.SuggestSanitizedMemberName(2, 2, "core");
  namer.ResolveMemberNamesForStruct(2, 4);
  EXPECT_THAT(namer.GetMemberName(2, 0), Eq("field0"));
  EXPECT_THAT(namer.GetMemberName(2, 1), Eq("apple"));
  EXPECT_THAT(namer.GetMemberName(2, 2), Eq("core"));
  EXPECT_THAT(namer.GetMemberName(2, 3), Eq("field3"));
}

TEST_F(SpvNamerTest,
       ResolveMemberNamesForStruct_GeneratedNameAvoidsConflictWithSuggestion) {
  Namer namer(fail_stream_);
  namer.SuggestSanitizedMemberName(2, 0, "field1");
  namer.ResolveMemberNamesForStruct(2, 2);
  EXPECT_THAT(namer.GetMemberName(2, 0), Eq("field1"));
  EXPECT_THAT(namer.GetMemberName(2, 1), Eq("field1_1"));
}

TEST_F(SpvNamerTest,
       ResolveMemberNamesForStruct_TruncatesOutOfBoundsSuggestion) {
  Namer namer(fail_stream_);
  namer.SuggestSanitizedMemberName(2, 3, "sitar");
  EXPECT_THAT(namer.GetMemberName(2, 3), Eq("sitar"));
  namer.ResolveMemberNamesForStruct(2, 2);
  EXPECT_THAT(namer.GetMemberName(2, 0), Eq("field0"));
  EXPECT_THAT(namer.GetMemberName(2, 1), Eq("field1"));
  EXPECT_THAT(namer.GetMemberName(2, 3), Eq(""));
}

}  // namespace
}  // namespace spirv
}  // namespace reader
}  // namespace tint
