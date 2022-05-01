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

#include <string>

#include "gtest/gtest.h"

#include "src/tint/fuzzers/tint_regex_fuzzer/wgsl_mutator.h"

namespace tint::fuzzers::regex_fuzzer {
namespace {

// Swaps two non-consecutive regions in the edge
TEST(SwapRegionsTest, SwapIntervalsEdgeNonConsecutive) {
    std::string R1 = ";region1;", R2 = ";regionregion2;", R3 = ";regionregionregion3;";
    std::string all_regions = R1 + R2 + R3;

    // this call should swap R1 with R3.
    SwapIntervals(0, R1.length(), R1.length() + R2.length(), R3.length(), all_regions);

    ASSERT_EQ(R3 + R2 + R1, all_regions);
}

// Swaps two non-consecutive regions not in the edge
TEST(SwapRegionsTest, SwapIntervalsNonConsecutiveNonEdge) {
    std::string R1 = ";region1;", R2 = ";regionregion2;", R3 = ";regionregionregion3;",
                R4 = ";regionregionregionregion4;", R5 = ";regionregionregionregionregion5;";
    std::string all_regions = R1 + R2 + R3 + R4 + R5;

    // this call should swap R2 with R4.
    SwapIntervals(R1.length(), R2.length(), R1.length() + R2.length() + R3.length(), R4.length(),
                  all_regions);

    ASSERT_EQ(R1 + R4 + R3 + R2 + R5, all_regions);
}

// Swaps two consecutive regions not in the edge (sorrounded by other
// regions)
TEST(SwapRegionsTest, SwapIntervalsConsecutiveEdge) {
    std::string R1 = ";region1;", R2 = ";regionregion2;", R3 = ";regionregionregion3;",
                R4 = ";regionregionregionregion4;", R5 = ";regionregionregionregionregion5;";
    std::string all_regions = R1 + R2 + R3 + R4;

    // this call should swap R2 with R3.
    SwapIntervals(R1.length(), R2.length(), R1.length() + R2.length(), R3.length(), all_regions);

    ASSERT_EQ(R1 + R3 + R2 + R4, all_regions);
}

// Swaps two consecutive regions not in the edge (not sorrounded by other
// regions)
TEST(SwapRegionsTest, SwapIntervalsConsecutiveNonEdge) {
    std::string R1 = ";region1;", R2 = ";regionregion2;", R3 = ";regionregionregion3;",
                R4 = ";regionregionregionregion4;", R5 = ";regionregionregionregionregion5;";
    std::string all_regions = R1 + R2 + R3 + R4 + R5;

    // this call should swap R4 with R5.
    SwapIntervals(R1.length() + R2.length() + R3.length(), R4.length(),
                  R1.length() + R2.length() + R3.length() + R4.length(), R5.length(), all_regions);

    ASSERT_EQ(R1 + R2 + R3 + R5 + R4, all_regions);
}

// Deletes the first region.
TEST(DeleteRegionTest, DeleteFirstRegion) {
    std::string R1 = ";region1;", R2 = ";regionregion2;", R3 = ";regionregionregion3;",
                R4 = ";regionregionregionregion4;", R5 = ";regionregionregionregionregion5;";
    std::string all_regions = R1 + R2 + R3 + R4 + R5;

    // This call should delete R1.
    DeleteInterval(0, R1.length(), all_regions);

    ASSERT_EQ(";" + R2 + R3 + R4 + R5, all_regions);
}

// Deletes the last region.
TEST(DeleteRegionTest, DeleteLastRegion) {
    std::string R1 = ";region1;", R2 = ";regionregion2;", R3 = ";regionregionregion3;",
                R4 = ";regionregionregionregion4;", R5 = ";regionregionregionregionregion5;";
    std::string all_regions = R1 + R2 + R3 + R4 + R5;

    // This call should delete R5.
    DeleteInterval(R1.length() + R2.length() + R3.length() + R4.length(), R5.length(), all_regions);

    ASSERT_EQ(R1 + R2 + R3 + R4 + ";", all_regions);
}

// Deletes the middle region.
TEST(DeleteRegionTest, DeleteMiddleRegion) {
    std::string R1 = ";region1;", R2 = ";regionregion2;", R3 = ";regionregionregion3;",
                R4 = ";regionregionregionregion4;", R5 = ";regionregionregionregionregion5;";
    std::string all_regions = R1 + R2 + R3 + R4 + R5;

    // This call should delete R3.
    DeleteInterval(R1.length() + R2.length(), R3.length(), all_regions);

    ASSERT_EQ(R1 + R2 + ";" + R4 + R5, all_regions);
}

TEST(InsertRegionTest, InsertRegionTest1) {
    std::string R1 = ";region1;", R2 = ";regionregion2;", R3 = ";regionregionregion3;",
                R4 = ";regionregionregionregion4;", R5 = ";regionregionregionregionregion5;";
    std::string all_regions = R1 + R2 + R3 + R4 + R5;

    // This call should insert R2 after R4.
    DuplicateInterval(R1.length(), R2.length(),
                      R1.length() + R2.length() + R3.length() + R4.length() - 1, all_regions);

    ASSERT_EQ(R1 + R2 + R3 + R4 + R2.substr(1, R2.size() - 1) + R5, all_regions);
}

TEST(InsertRegionTest, InsertRegionTest2) {
    std::string R1 = ";region1;", R2 = ";regionregion2;", R3 = ";regionregionregion3;",
                R4 = ";regionregionregionregion4;", R5 = ";regionregionregionregionregion5;";

    std::string all_regions = R1 + R2 + R3 + R4 + R5;

    // This call should insert R3 after R1.
    DuplicateInterval(R1.length() + R2.length(), R3.length(), R1.length() - 1, all_regions);

    ASSERT_EQ(R1 + R3.substr(1, R3.length() - 1) + R2 + R3 + R4 + R5, all_regions);
}

TEST(InsertRegionTest, InsertRegionTest3) {
    std::string R1 = ";region1;", R2 = ";regionregion2;", R3 = ";regionregionregion3;",
                R4 = ";regionregionregionregion4;", R5 = ";regionregionregionregionregion5;";

    std::string all_regions = R1 + R2 + R3 + R4 + R5;

    // This call should insert R2 after R5.
    DuplicateInterval(R1.length(), R2.length(), all_regions.length() - 1, all_regions);

    ASSERT_EQ(R1 + R2 + R3 + R4 + R5 + R2.substr(1, R2.length() - 1), all_regions);
}

TEST(ReplaceIdentifierTest, ReplaceIdentifierTest1) {
    std::string R1 = "|region1|", R2 = "; region2;", R3 = "---------region3---------",
                R4 = "++region4++", R5 = "***region5***";
    std::string all_regions = R1 + R2 + R3 + R4 + R5;

    // Replaces R3 with R1.
    ReplaceRegion(0, R1.length(), R1.length() + R2.length(), R3.length(), all_regions);

    ASSERT_EQ(R1 + R2 + R1 + R4 + R5, all_regions);
}

TEST(ReplaceIdentifierTest, ReplaceIdentifierTest2) {
    std::string R1 = "|region1|", R2 = "; region2;", R3 = "---------region3---------",
                R4 = "++region4++", R5 = "***region5***";
    std::string all_regions = R1 + R2 + R3 + R4 + R5;

    // Replaces R5 with R3.
    ReplaceRegion(R1.length() + R2.length(), R3.length(),
                  R1.length() + R2.length() + R3.length() + R4.length(), R5.length(), all_regions);

    ASSERT_EQ(R1 + R2 + R3 + R4 + R3, all_regions);
}

TEST(GetIdentifierTest, GetIdentifierTest1) {
    std::string wgsl_code =
        R"(fn clamp_0acf8f() {
        var res: vec2<f32> = clamp(vec2<f32>(), vec2<f32>(), vec2<f32>());
      }
      @stage(vertex)
      fn vertex_main() -> @builtin(position) vec4<f32> {
         clamp_0acf8f();"
         return vec4<f32>();
      }
      @stage(fragment)
      fn fragment_main() {
        clamp_0acf8f();
      }
      @stage(compute) @workgroup_size(1)
      fn compute_main() {"
        var<private> foo: f32 = 0.0;
        clamp_0acf8f();
      })";

    std::vector<std::pair<size_t, size_t>> identifiers_pos = GetIdentifiers(wgsl_code);

    std::vector<std::pair<size_t, size_t>> ground_truth = {
        std::make_pair(3, 12),   std::make_pair(28, 3),  std::make_pair(37, 4),
        std::make_pair(49, 5),   std::make_pair(60, 3),  std::make_pair(68, 4),
        std::make_pair(81, 4),   std::make_pair(110, 5), std::make_pair(130, 2),
        std::make_pair(140, 4),  std::make_pair(151, 7), std::make_pair(169, 4),
        std::make_pair(190, 12), std::make_pair(216, 6), std::make_pair(228, 3),
        std::make_pair(251, 5),  std::make_pair(273, 2), std::make_pair(285, 4),
        std::make_pair(302, 12), std::make_pair(333, 5), std::make_pair(349, 14),
        std::make_pair(373, 2),  std::make_pair(384, 4), std::make_pair(402, 3),
        std::make_pair(415, 3),  std::make_pair(420, 3), std::make_pair(439, 12)};

    ASSERT_EQ(ground_truth, identifiers_pos);
}

TEST(TestGetLiteralsValues, TestGetLiteralsValues1) {
    std::string wgsl_code =
        R"(fn clamp_0acf8f() {
        var res: vec2<f32> = clamp(vec2<f32>(), vec2<f32>(), vec2<f32>());
      }
      @stage(vertex)
      fn vertex_main() -> @builtin(position) vec4<f32> {
        clamp_0acf8f();
        var foo_1: i32 = 3;
        return vec4<f32>();
      }
      @stage(fragment)
      fn fragment_main() {
        clamp_0acf8f();
      }
      @stage(compute) @workgroup_size(1)
      fn compute_main() {
        var<private> foo: f32 = 0.0;
        var foo_2: i32 = 10;
        clamp_0acf8f();
      }
      foo_1 = 5 + 7;
      var foo_3 : i32 = -20;)";

    std::vector<std::pair<size_t, size_t>> literals_pos = GetIntLiterals(wgsl_code);

    std::vector<std::string> ground_truth = {"3", "10", "5", "7", "-20"};

    std::vector<std::string> result;

    for (auto pos : literals_pos) {
        result.push_back(wgsl_code.substr(pos.first, pos.second));
    }

    ASSERT_EQ(ground_truth, result);
}

TEST(InsertReturnTest, FindClosingBrace) {
    std::string wgsl_code =
        R"(fn clamp_0acf8f() {
        if(false){

        } else{
          var res: vec2<f32> = clamp(vec2<f32>(), vec2<f32>(), vec2<f32>());
          }
        }
        @stage(vertex)
        fn vertex_main() -> @builtin(position) vec4<f32> {
          clamp_0acf8f();
          var foo_1: i32 = 3;
          return vec4<f32>();
        }
        @stage(fragment)
        fn fragment_main() {
          clamp_0acf8f();
        }
        @stage(compute) @workgroup_size(1)
        fn compute_main() {
          var<private> foo: f32 = 0.0;
          var foo_2: i32 = 10;
          clamp_0acf8f();
        }
        foo_1 = 5 + 7;
        var foo_3 : i32 = -20;
      )";
    size_t opening_bracket_pos = 18;
    size_t closing_bracket_pos = FindClosingBrace(opening_bracket_pos, wgsl_code);

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
    std::string wgsl_code =
        R"(fn clamp_0acf8f() {
      // This comment } causes the test to fail.
      "if(false){

      } else{
        var res: vec2<f32> = clamp(vec2<f32>(), vec2<f32>(), vec2<f32>());
        }
      }
      @stage(vertex)
      fn vertex_main() -> @builtin(position) vec4<f32> {
        clamp_0acf8f();
        var foo_1: i32 = 3;
        return vec4<f32>();
      }
      @stage(fragment)
      fn fragment_main() {
        clamp_0acf8f();
      }
      @stage(compute) @workgroup_size(1)
      fn compute_main() {
        var<private> foo: f32 = 0.0;
        var foo_2: i32 = 10;
        clamp_0acf8f();
      }
      foo_1 = 5 + 7;
      var foo_3 : i32 = -20;)";
    size_t opening_bracket_pos = 18;
    size_t closing_bracket_pos = FindClosingBrace(opening_bracket_pos, wgsl_code);

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

TEST(TestInsertReturn, TestInsertReturn1) {
    std::string wgsl_code =
        R"(fn clamp_0acf8f() {
        var res: vec2<f32> = clamp(vec2<f32>(), vec2<f32>(), vec2<f32>());
      }
      @stage(vertex)
      fn vertex_main() -> @builtin(position) vec4<f32> {
        clamp_0acf8f();
        var foo_1: i32 = 3;
        return vec4<f32>();
      }
      @stage(fragment)
      fn fragment_main() {
        clamp_0acf8f();
      }
      @stage(compute) @workgroup_size(1)
      fn compute_main() {
        var<private> foo: f32 = 0.0;
        var foo_2: i32 = 10;
        clamp_0acf8f();
      }
      foo_1 = 5 + 7;
      var foo_3 : i32 = -20;)";

    std::vector<size_t> semicolon_pos;
    for (size_t pos = wgsl_code.find(";", 0); pos != std::string::npos;
         pos = wgsl_code.find(";", pos + 1)) {
        semicolon_pos.push_back(pos);
    }

    // should insert a return true statement after the first semicolon of the
    // first function the the WGSL-like string above.
    wgsl_code.insert(semicolon_pos[0] + 1, "return true;");

    std::string expected_wgsl_code =
        R"(fn clamp_0acf8f() {
        var res: vec2<f32> = clamp(vec2<f32>(), vec2<f32>(), vec2<f32>());return true;
      }
      @stage(vertex)
      fn vertex_main() -> @builtin(position) vec4<f32> {
        clamp_0acf8f();
        var foo_1: i32 = 3;
        return vec4<f32>();
      }
      @stage(fragment)
      fn fragment_main() {
        clamp_0acf8f();
      }
      @stage(compute) @workgroup_size(1)
      fn compute_main() {
        var<private> foo: f32 = 0.0;
        var foo_2: i32 = 10;
        clamp_0acf8f();
      }
      foo_1 = 5 + 7;
      var foo_3 : i32 = -20;)";

    ASSERT_EQ(expected_wgsl_code, wgsl_code);
}

TEST(TestInsertReturn, TestFunctionPositions) {
    std::string wgsl_code =
        R"(fn clamp_0acf8f() {
          var res: vec2<f32> = clamp(vec2<f32>(), vec2<f32>(), vec2<f32>());
        }
        @stage(vertex)
        fn vertex_main() -> @builtin(position) vec4<f32> {
          clamp_0acf8f();
          var foo_1: i32 = 3;
          return vec4<f32>();
        }
        @stage(fragment)
        fn fragment_main() {
          clamp_0acf8f();
        }
        @stage(compute) @workgroup_size(1)
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

    std::vector<size_t> function_positions = GetFunctionBodyPositions(wgsl_code);
    std::vector<size_t> expected_positions = {187, 607};
    ASSERT_EQ(expected_positions, function_positions);
}

TEST(TestInsertReturn, TestMissingSemicolon) {
    std::string wgsl_code =
        R"(fn clamp_0acf8f() {
          var res: vec2<f32> = clamp(vec2<f32>(), vec2<f32>(), vec2<f32>())
        }
        @stage(vertex)
        fn vertex_main() -> @builtin(position) vec4<f32> {
          clamp_0acf8f()
          var foo_1: i32 = 3
          return vec4<f32>()
        }
        @stage(fragment)
        fn fragment_main() {
          clamp_0acf8f();
        }
        @stage(compute) @workgroup_size(1)
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

    RandomGenerator generator(0);
    InsertReturnStatement(wgsl_code, generator);

    // No semicolons found in the function's body, so wgsl_code
    // should remain unchanged.
    std::string expected_wgsl_code =
        R"(fn clamp_0acf8f() {
          var res: vec2<f32> = clamp(vec2<f32>(), vec2<f32>(), vec2<f32>())
        }
        @stage(vertex)
        fn vertex_main() -> @builtin(position) vec4<f32> {
          clamp_0acf8f()
          var foo_1: i32 = 3
          return vec4<f32>()
        }
        @stage(fragment)
        fn fragment_main() {
          clamp_0acf8f();
        }
        @stage(compute) @workgroup_size(1)
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

}  // namespace
}  // namespace tint::fuzzers::regex_fuzzer
