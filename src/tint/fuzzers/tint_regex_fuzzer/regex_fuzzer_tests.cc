// Copyright 2021 The Tint Authors.
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

#include <optional>
#include <string>

#include "gtest/gtest.h"

#include "src/tint/fuzzers/tint_regex_fuzzer/wgsl_mutator.h"

namespace tint::fuzzers::regex_fuzzer {
namespace {

class WgslMutatorTest : public WgslMutator {
  public:
    explicit WgslMutatorTest(RandomGenerator& generator) : WgslMutator(generator) {}

    using WgslMutator::DeleteInterval;
    using WgslMutator::DuplicateInterval;
    using WgslMutator::FindClosingBracket;
    using WgslMutator::FindOperatorOccurrence;
    using WgslMutator::GetFunctionBodyPositions;
    using WgslMutator::GetFunctionCallIdentifiers;
    using WgslMutator::GetIdentifiers;
    using WgslMutator::GetIntLiterals;
    using WgslMutator::GetLoopBodyPositions;
    using WgslMutator::GetSwizzles;
    using WgslMutator::GetVectorInitializers;
    using WgslMutator::ReplaceRegion;
    using WgslMutator::SwapIntervals;
};

// Swaps two non-consecutive regions in the edge
TEST(SwapRegionsTest, SwapIntervalsEdgeNonConsecutive) {
    RandomGenerator generator(0);
    WgslMutatorTest mutator(generator);
    std::string R1 = ";region1;", R2 = ";regionregion2;", R3 = ";regionregionregion3;";
    std::string all_regions = R1 + R2 + R3;

    // this call should swap R1 with R3.
    mutator.SwapIntervals(0, R1.length(), R1.length() + R2.length(), R3.length(), all_regions);

    ASSERT_EQ(R3 + R2 + R1, all_regions);
}

// Swaps two non-consecutive regions not in the edge
TEST(SwapRegionsTest, SwapIntervalsNonConsecutiveNonEdge) {
    RandomGenerator generator(0);
    WgslMutatorTest mutator(generator);
    std::string R1 = ";region1;", R2 = ";regionregion2;", R3 = ";regionregionregion3;",
                R4 = ";regionregionregionregion4;", R5 = ";regionregionregionregionregion5;";
    std::string all_regions = R1 + R2 + R3 + R4 + R5;

    // this call should swap R2 with R4.
    mutator.SwapIntervals(R1.length(), R2.length(), R1.length() + R2.length() + R3.length(),
                          R4.length(), all_regions);

    ASSERT_EQ(R1 + R4 + R3 + R2 + R5, all_regions);
}

// Swaps two consecutive regions not in the edge (sorrounded by other
// regions)
TEST(SwapRegionsTest, SwapIntervalsConsecutiveEdge) {
    RandomGenerator generator(0);
    WgslMutatorTest mutator(generator);
    std::string R1 = ";region1;", R2 = ";regionregion2;", R3 = ";regionregionregion3;",
                R4 = ";regionregionregionregion4;", R5 = ";regionregionregionregionregion5;";
    std::string all_regions = R1 + R2 + R3 + R4;

    // this call should swap R2 with R3.
    mutator.SwapIntervals(R1.length(), R2.length(), R1.length() + R2.length(), R3.length(),
                          all_regions);

    ASSERT_EQ(R1 + R3 + R2 + R4, all_regions);
}

// Swaps two consecutive regions not in the edge (not sorrounded by other
// regions)
TEST(SwapRegionsTest, SwapIntervalsConsecutiveNonEdge) {
    RandomGenerator generator(0);
    WgslMutatorTest mutator(generator);
    std::string R1 = ";region1;", R2 = ";regionregion2;", R3 = ";regionregionregion3;",
                R4 = ";regionregionregionregion4;", R5 = ";regionregionregionregionregion5;";
    std::string all_regions = R1 + R2 + R3 + R4 + R5;

    // this call should swap R4 with R5.
    mutator.SwapIntervals(R1.length() + R2.length() + R3.length(), R4.length(),
                          R1.length() + R2.length() + R3.length() + R4.length(), R5.length(),
                          all_regions);

    ASSERT_EQ(R1 + R2 + R3 + R5 + R4, all_regions);
}

// Deletes the first region.
TEST(DeleteRegionTest, DeleteFirstRegion) {
    RandomGenerator generator(0);
    WgslMutatorTest mutator(generator);
    std::string R1 = ";region1;", R2 = ";regionregion2;", R3 = ";regionregionregion3;",
                R4 = ";regionregionregionregion4;", R5 = ";regionregionregionregionregion5;";
    std::string all_regions = R1 + R2 + R3 + R4 + R5;

    // This call should delete R1.
    mutator.DeleteInterval(0, R1.length(), all_regions);

    ASSERT_EQ(";" + R2 + R3 + R4 + R5, all_regions);
}

// Deletes the last region.
TEST(DeleteRegionTest, DeleteLastRegion) {
    RandomGenerator generator(0);
    WgslMutatorTest mutator(generator);
    std::string R1 = ";region1;", R2 = ";regionregion2;", R3 = ";regionregionregion3;",
                R4 = ";regionregionregionregion4;", R5 = ";regionregionregionregionregion5;";
    std::string all_regions = R1 + R2 + R3 + R4 + R5;

    // This call should delete R5.
    mutator.DeleteInterval(R1.length() + R2.length() + R3.length() + R4.length(), R5.length(),
                           all_regions);

    ASSERT_EQ(R1 + R2 + R3 + R4 + ";", all_regions);
}

// Deletes the middle region.
TEST(DeleteRegionTest, DeleteMiddleRegion) {
    RandomGenerator generator(0);
    WgslMutatorTest mutator(generator);
    std::string R1 = ";region1;", R2 = ";regionregion2;", R3 = ";regionregionregion3;",
                R4 = ";regionregionregionregion4;", R5 = ";regionregionregionregionregion5;";
    std::string all_regions = R1 + R2 + R3 + R4 + R5;

    // This call should delete R3.
    mutator.DeleteInterval(R1.length() + R2.length(), R3.length(), all_regions);

    ASSERT_EQ(R1 + R2 + ";" + R4 + R5, all_regions);
}

TEST(InsertRegionTest, InsertRegionTest1) {
    RandomGenerator generator(0);
    WgslMutatorTest mutator(generator);
    std::string R1 = ";region1;", R2 = ";regionregion2;", R3 = ";regionregionregion3;",
                R4 = ";regionregionregionregion4;", R5 = ";regionregionregionregionregion5;";
    std::string all_regions = R1 + R2 + R3 + R4 + R5;

    // This call should insert R2 after R4.
    mutator.DuplicateInterval(R1.length(), R2.length(),
                              R1.length() + R2.length() + R3.length() + R4.length() - 1,
                              all_regions);

    ASSERT_EQ(R1 + R2 + R3 + R4 + R2.substr(1, R2.size() - 1) + R5, all_regions);
}

TEST(InsertRegionTest, InsertRegionTest2) {
    RandomGenerator generator(0);
    WgslMutatorTest mutator(generator);
    std::string R1 = ";region1;", R2 = ";regionregion2;", R3 = ";regionregionregion3;",
                R4 = ";regionregionregionregion4;", R5 = ";regionregionregionregionregion5;";

    std::string all_regions = R1 + R2 + R3 + R4 + R5;

    // This call should insert R3 after R1.
    mutator.DuplicateInterval(R1.length() + R2.length(), R3.length(), R1.length() - 1, all_regions);

    ASSERT_EQ(R1 + R3.substr(1, R3.length() - 1) + R2 + R3 + R4 + R5, all_regions);
}

TEST(InsertRegionTest, InsertRegionTest3) {
    RandomGenerator generator(0);
    WgslMutatorTest mutator(generator);
    std::string R1 = ";region1;", R2 = ";regionregion2;", R3 = ";regionregionregion3;",
                R4 = ";regionregionregionregion4;", R5 = ";regionregionregionregionregion5;";

    std::string all_regions = R1 + R2 + R3 + R4 + R5;

    // This call should insert R2 after R5.
    mutator.DuplicateInterval(R1.length(), R2.length(), all_regions.length() - 1, all_regions);

    ASSERT_EQ(R1 + R2 + R3 + R4 + R5 + R2.substr(1, R2.length() - 1), all_regions);
}

TEST(ReplaceIdentifierTest, ReplaceIdentifierTest1) {
    RandomGenerator generator(0);
    WgslMutatorTest mutator(generator);
    std::string R1 = "|region1|", R2 = "; region2;", R3 = "---------region3---------",
                R4 = "++region4++", R5 = "***region5***";
    std::string all_regions = R1 + R2 + R3 + R4 + R5;

    // Replaces R3 with R1.
    mutator.ReplaceRegion(0, R1.length(), R1.length() + R2.length(), R3.length(), all_regions);

    ASSERT_EQ(R1 + R2 + R1 + R4 + R5, all_regions);
}

TEST(ReplaceIdentifierTest, ReplaceIdentifierTest2) {
    RandomGenerator generator(0);
    WgslMutatorTest mutator(generator);
    std::string R1 = "|region1|", R2 = "; region2;", R3 = "---------region3---------",
                R4 = "++region4++", R5 = "***region5***";
    std::string all_regions = R1 + R2 + R3 + R4 + R5;

    // Replaces R5 with R3.
    mutator.ReplaceRegion(R1.length() + R2.length(), R3.length(),
                          R1.length() + R2.length() + R3.length() + R4.length(), R5.length(),
                          all_regions);

    ASSERT_EQ(R1 + R2 + R3 + R4 + R3, all_regions);
}

TEST(GetIdentifierTest, GetIdentifierTest1) {
    RandomGenerator generator(0);
    WgslMutatorTest mutator(generator);
    std::string wgsl_code =
        R"(fn clamp_0acf8f() {
        var res: vec2<f32> = clamp(vec2<f32>(), vec2<f32>(), vec2<f32>());
      }
      @vertex
      fn vertex_main() -> @builtin(position) vec4<f32> {
         clamp_0acf8f();"
         return vec4<f32>();
      }
      @fragment
      fn fragment_main() {
        clamp_0acf8f();
      }
      @compute @workgroup_size(1)
      fn compute_main() {"
        var<private> foo: f32 = 0.0;
        clamp_0acf8f();
      })";

    std::vector<std::pair<size_t, size_t>> identifiers_pos = mutator.GetIdentifiers(wgsl_code);
    std::vector<std::pair<size_t, size_t>> ground_truth = {
        {3, 12},   {32, 3},   {49, 5},   {126, 11}, {144, 7}, {152, 8}, {183, 12},
        {262, 13}, {288, 12}, {328, 14}, {355, 12}, {385, 7}, {394, 3}, {418, 12}};
    ASSERT_EQ(ground_truth, identifiers_pos);
}

TEST(TestGetLiteralsValues, TestGetLiteralsValues1) {
    RandomGenerator generator(0);
    WgslMutatorTest mutator(generator);
    std::string wgsl_code =
        R"(fn clamp_0acf8f() {
        var res: vec2<f32> = clamp(vec2<f32>(), vec2<f32>(), vec2<f32>());
      }
      @vertex
      fn vertex_main() -> @builtin(position) vec4<f32> {
        clamp_0acf8f();
        var foo_1: i32 = 3;
        return vec4<f32>();
      }
      @fragment
      fn fragment_main() {
        clamp_0acf8f();
      }
      @compute @workgroup_size(1)
      fn compute_main() {
        var<private> foo: f32 = 0.0;
        var foo_2: i32 = 10;
        clamp_0acf8f();
      }
      foo_1 = 5 + 7;
      var foo_3 : i32 = -20;)";

    std::vector<std::pair<size_t, size_t>> literals_pos = mutator.GetIntLiterals(wgsl_code);

    std::vector<std::string> ground_truth = {"3", "10", "5", "7", "-20"};

    std::vector<std::string> result;

    for (auto pos : literals_pos) {
        result.push_back(wgsl_code.substr(pos.first, pos.second));
    }

    ASSERT_EQ(ground_truth, result);
}

TEST(InsertReturnTest, FindClosingBrace) {
    RandomGenerator generator(0);
    WgslMutatorTest mutator(generator);
    std::string wgsl_code =
        R"(fn clamp_0acf8f() {
        if(false){

        } else{
          var res: vec2<f32> = clamp(vec2<f32>(), vec2<f32>(), vec2<f32>());
          }
        }
        @vertex
        fn vertex_main() -> @builtin(position) vec4<f32> {
          clamp_0acf8f();
          var foo_1: i32 = 3;
          return vec4<f32>();
        }
        @fragment
        fn fragment_main() {
          clamp_0acf8f();
        }
        @compute @workgroup_size(1)
        fn compute_main() {
          var<private> foo: f32 = 0.0;
          var foo_2: i32 = 10;
          clamp_0acf8f();
        }
        foo_1 = 5 + 7;
        var foo_3 : i32 = -20;
      )";
    size_t opening_bracket_pos = 18;
    size_t closing_bracket_pos =
        mutator.FindClosingBracket(opening_bracket_pos, wgsl_code, '{', '}');

    // The -1 is needed since the function body starts after the left bracket.
    std::string function_body =
        wgsl_code.substr(opening_bracket_pos + 1, closing_bracket_pos - opening_bracket_pos - 1);
    std::string expected =
        R"(
        if(false){

        } else{
          var res: vec2<f32> = clamp(vec2<f32>(), vec2<f32>(), vec2<f32>());
          }
        )";
    ASSERT_EQ(expected, function_body);
}

TEST(InsertReturnTest, FindClosingBraceFailing) {
    RandomGenerator generator(0);
    WgslMutatorTest mutator(generator);
    std::string wgsl_code =
        R"(fn clamp_0acf8f() {
      // This comment } causes the test to fail.
      "if(false){

      } else{
        var res: vec2<f32> = clamp(vec2<f32>(), vec2<f32>(), vec2<f32>());
        }
      }
      @vertex
      fn vertex_main() -> @builtin(position) vec4<f32> {
        clamp_0acf8f();
        var foo_1: i32 = 3;
        return vec4<f32>();
      }
      @fragment
      fn fragment_main() {
        clamp_0acf8f();
      }
      @compute @workgroup_size(1)
      fn compute_main() {
        var<private> foo: f32 = 0.0;
        var foo_2: i32 = 10;
        clamp_0acf8f();
      }
      foo_1 = 5 + 7;
      var foo_3 : i32 = -20;)";
    size_t opening_bracket_pos = 18;
    size_t closing_bracket_pos =
        mutator.FindClosingBracket(opening_bracket_pos, wgsl_code, '{', '}');

    // The -1 is needed since the function body starts after the left bracket.
    std::string function_body =
        wgsl_code.substr(opening_bracket_pos + 1, closing_bracket_pos - opening_bracket_pos - 1);
    std::string expected =
        R"(// This comment } causes the test to fail.
      "if(false){

      } else{
        var res: vec2<f32> = clamp(vec2<f32>(), vec2<f32>(), vec2<f32>());
        })";
    ASSERT_NE(expected, function_body);
}

TEST(TestInsertReturn, TestFunctionPositions1) {
    RandomGenerator generator(0);
    WgslMutatorTest mutator(generator);
    std::string wgsl_code =
        R"(fn clamp_0acf8f() {
          var res: vec2<f32> = clamp(vec2<f32>(), vec2<f32>(), vec2<f32>());
        }
        @vertex
        fn vertex_main() -> @builtin(position) vec4<f32> {
          clamp_0acf8f();
          var foo_1: i32 = 3;
          return vec4<f32>();
        }
        @fragment
        fn fragment_main() {
          clamp_0acf8f();
        }
        @compute @workgroup_size(1)
        fn compute_main() {
          var<private> foo: f32 = 0.0;
          var foo_2: i32 = 10;
          clamp_0acf8f();
        }
        fn vert_main() -> @builtin(position) vec4<f32> {
          clamp_0acf8f();
          var foo_1: i32 = 3;
          return vec4<f32>();
        }
        foo_1 = 5 + 7;
        var foo_3 : i32 = -20;)";

    std::vector<std::pair<size_t, bool>> function_positions =
        mutator.GetFunctionBodyPositions(wgsl_code);
    std::vector<std::pair<size_t, bool>> expected_positions = {
        {18, false}, {180, true}, {323, false}, {423, false}, {586, true}};
    ASSERT_EQ(expected_positions, function_positions);
}

TEST(TestInsertReturn, TestFunctionPositions2) {
    RandomGenerator generator(0);
    WgslMutatorTest mutator(generator);
    std::string wgsl_code =
        R"(fn some_loop_body() {
}

fn f() {
  var j : i32; i = (i + 1)) {
    some_loop_body(); ((i < 5) && (j < 10));
  for(var i : i32 = 0;
    j = (i * 30);
  }
}
)";

    std::vector<std::pair<size_t, bool>> function_positions =
        mutator.GetFunctionBodyPositions(wgsl_code);
    std::vector<std::pair<size_t, bool>> expected_positions = {{20, false}, {32, false}};
    ASSERT_EQ(expected_positions, function_positions);
}

TEST(TestInsertReturn, TestMissingSemicolon) {
    RandomGenerator generator(0);
    WgslMutatorTest mutator(generator);
    std::string wgsl_code =
        R"(fn clamp_0acf8f() {
          var res: vec2<f32> = clamp(vec2<f32>(), vec2<f32>(), vec2<f32>())
        }
        @vertex
        fn vertex_main() -> @builtin(position) vec4<f32> {
          clamp_0acf8f()
          var foo_1: i32 = 3
          return vec4<f32>()
        }
        @fragment
        fn fragment_main() {
          clamp_0acf8f();
        }
        @compute @workgroup_size(1)
        fn compute_main() {
          var<private> foo: f32 = 0.0;
          var foo_2: i32 = 10;
          clamp_0acf8f();
        }
        fn vert_main() -> @builtin(position) vec4<f32> {
          clamp_0acf8f()
          var foo_1: i32 = 3
          return vec4<f32>()
        }
        foo_1 = 5 + 7;
        var foo_3 : i32 = -20;)";

    mutator.InsertReturnStatement(wgsl_code);

    // No semicolons found in the function's body, so wgsl_code
    // should remain unchanged.
    std::string expected_wgsl_code =
        R"(fn clamp_0acf8f() {
          var res: vec2<f32> = clamp(vec2<f32>(), vec2<f32>(), vec2<f32>())
        }
        @vertex
        fn vertex_main() -> @builtin(position) vec4<f32> {
          clamp_0acf8f()
          var foo_1: i32 = 3
          return vec4<f32>()
        }
        @fragment
        fn fragment_main() {
          clamp_0acf8f();
        }
        @compute @workgroup_size(1)
        fn compute_main() {
          var<private> foo: f32 = 0.0;
          var foo_2: i32 = 10;
          clamp_0acf8f();
        }
        fn vert_main() -> @builtin(position) vec4<f32> {
          clamp_0acf8f()
          var foo_1: i32 = 3
          return vec4<f32>()
        }
        foo_1 = 5 + 7;
        var foo_3 : i32 = -20;)";
    ASSERT_EQ(expected_wgsl_code, wgsl_code);
}

TEST(TestReplaceOperator, TestIdentifyOperators) {
    RandomGenerator generator(0);
    WgslMutatorTest mutator(generator);
    std::string code =
        R"(
x += 2;
y = a + b;
z = -a;
x *= b / c;
t = t && t | t || t;
b = b > c ^ c <= d;
a >>= b;
b <<= a;
a = a << 2;
b = b >> 3;
c = a % 3;
d %= e;
)";
    // These are the operator occurrences that will be observed by going through the file character
    // by character. This includes, for example, identifying the ">" operator if search starts after
    // the first character of ">>".
    std::vector<std::pair<uint32_t, uint32_t>> operator_occurrences = {
        {3, 2},   {4, 1},   {11, 1},  {15, 1},  {22, 1},  {24, 1},  {30, 2},  {31, 1},
        {35, 1},  {42, 1},  {46, 2},  {47, 1},  {51, 1},  {55, 2},  {56, 1},  {63, 1},
        {67, 1},  {71, 1},  {75, 2},  {76, 1},  {83, 3},  {84, 2},  {85, 1},  {92, 3},
        {93, 2},  {94, 1},  {101, 1}, {105, 2}, {106, 1}, {113, 1}, {117, 2}, {118, 1},
        {125, 1}, {129, 1}, {136, 2}, {137, 1}, {3, 2}};
    uint32_t operator_occurrence_index = 0;
    for (size_t i = 0; i < code.length(); i++) {
        // Move on to the next operator occurrence if the current index into the code string exceeds
        // the index associated with that operator occurrence. Exception: stay with the last
        // operator occurrence if search has already passed the last operator in the file.
        if (i < code.length() - 2 && i > operator_occurrences[operator_occurrence_index].first) {
            operator_occurrence_index =
                (operator_occurrence_index + 1) % operator_occurrences.size();
        }
        ASSERT_EQ(operator_occurrences[operator_occurrence_index],
                  mutator.FindOperatorOccurrence(code, static_cast<uint32_t>(i)).value());
    }
}

TEST(TestReplaceOperator, TestFindOperatorOccurrenceOnSmallStrings) {
    RandomGenerator generator(0);
    WgslMutatorTest mutator(generator);
    ASSERT_FALSE(mutator.FindOperatorOccurrence("", 0).has_value());
    ASSERT_FALSE(mutator.FindOperatorOccurrence(" ", 0).has_value());
    ASSERT_FALSE(mutator.FindOperatorOccurrence("  ", 0).has_value());
}

TEST(TestInsertBreakOrContinue, TestLoopPositions1) {
    RandomGenerator generator(0);
    WgslMutatorTest mutator(generator);
    std::string wgsl_code = " loop { } loop { } loop { }";
    std::vector<size_t> loop_positions = mutator.GetLoopBodyPositions(wgsl_code);
    std::vector<size_t> expected_positions = {6, 15, 24};
    ASSERT_EQ(expected_positions, loop_positions);
}

TEST(TestInsertBreakOrContinue, TestLoopPositions2) {
    RandomGenerator generator(0);
    WgslMutatorTest mutator(generator);
    std::string wgsl_code = R"( loop { } loop
{ } loop { })";
    std::vector<size_t> loop_positions = mutator.GetLoopBodyPositions(wgsl_code);
    std::vector<size_t> expected_positions = {6, 15, 24};
    ASSERT_EQ(expected_positions, loop_positions);
}

TEST(TestInsertBreakOrContinue, TestLoopPositions3) {
    RandomGenerator generator(0);
    WgslMutatorTest mutator(generator);
    // This WGSL-like code is not valid, but it suffices to test regex-based matching (which is
    // intended to work well on semi-valid code).
    std::string wgsl_code =
        R"(fn compute_main() {
  loop {
    var twice: i32 = 2 * i;
    i++;
    if i == 5 { break; }
      loop
      {
      var twice: i32 = 2 * i;
      i++;
      while (i < 100) { i++; }
      if i == 5 { break; }
    }
  }
  for (a = 0; a < 100; a++)   {
    if (a > 50) {
      break;
    }
      while (i < 100) { i++; }
  }
})";

    std::vector<size_t> loop_positions = mutator.GetLoopBodyPositions(wgsl_code);
    std::vector<size_t> expected_positions = {27, 108, 173, 249, 310};
    ASSERT_EQ(expected_positions, loop_positions);
}

TEST(TestInsertBreakOrContinue, TestLoopPositions4) {
    RandomGenerator generator(0);
    WgslMutatorTest mutator(generator);
    std::string wgsl_code =
        R"(fn clamp_0acf8f() {
        var res: vec2<f32> = clamp(vec2<f32>(), vec2<f32>(), vec2<f32>());
      }
      @vertex
      fn vertex_main() -> @builtin(position) vec4<f32> {
         clamp_0acf8f();"
         return vec4<f32>();
      }
      @fragment
      fn fragment_main() {
        clamp_0acf8f();
      }
      @compute @workgroup_size(1)
      fn compute_main() {"
        var<private> foo: f32 = 0.0;
        clamp_0acf8f    ();
      })";

    std::vector<size_t> loop_positions = mutator.GetLoopBodyPositions(wgsl_code);
    ASSERT_TRUE(loop_positions.empty());
}

TEST(TestReplaceFunctionCallWithBuiltin, FindFunctionCalls) {
    RandomGenerator generator(0);
    WgslMutatorTest mutator(generator);
    std::string function_body = R"({
          var<private> foo: f32 = 0.0;
          var foo_2: i32 = 10;
          clamp_0acf8f  ();
          _0acf8f();
          f
();
          j = (i * 30);
        })";
    std::vector<std::pair<size_t, size_t>> call_identifiers =
        mutator.GetFunctionCallIdentifiers(function_body);
    std::vector<std::pair<size_t, size_t>> ground_truth{{82, 12}, {110, 7}, {131, 1}};
    ASSERT_EQ(ground_truth, call_identifiers);
}

TEST(TestAddSwizzle, FindSwizzles) {
    RandomGenerator generator(0);
    WgslMutatorTest mutator(generator);
    std::string code = R"(x
v.xxyy.wz.x;
u.rgba.rrg.b)";
    std::vector<std::pair<size_t, size_t>> swizzles = mutator.GetSwizzles(code);
    std::vector<std::pair<size_t, size_t>> ground_truth{{3, 5},  {8, 3},  {11, 2},
                                                        {16, 5}, {21, 4}, {25, 2}};
    ASSERT_EQ(ground_truth, swizzles);
}

TEST(TestAddSwizzle, FindVectorInitializers) {
    RandomGenerator generator(0);
    WgslMutatorTest mutator(generator);
    std::string code = R"(
vec4<f32>(vec2<f32>(1, 2), vec2<f32>(3))

vec2<i32>(1, abs(abs(2)))
)";
    std::vector<std::pair<size_t, size_t>> swizzles = mutator.GetVectorInitializers(code);
    std::vector<std::pair<size_t, size_t>> ground_truth{{1, 40}, {11, 15}, {28, 12}, {43, 25}};
    ASSERT_EQ(ground_truth, swizzles);
}

}  // namespace
}  // namespace tint::fuzzers::regex_fuzzer
