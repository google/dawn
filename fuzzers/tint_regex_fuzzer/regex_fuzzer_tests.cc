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

#include "fuzzers/tint_regex_fuzzer/wgsl_mutator.h"

namespace tint {
namespace fuzzers {
namespace regex_fuzzer {
namespace {

// Swaps two non-consecutive regions in the edge
TEST(SwapRegionsTest, SwapIntervalsEdgeNonConsecutive) {
  std::string R1 = "|region1|", R2 = "; region2;",
              R3 = "---------region3---------";
  std::string all_regions = R1 + R2 + R3;

  // this call should swap R1 with R3.
  SwapIntervals(0, R1.length() - 1, R1.length() + R2.length(),
                all_regions.length() - 1, &all_regions);

  ASSERT_EQ(R3 + R2 + R1, all_regions);
}

// Swaps two non-consecutive regions not in the edge
TEST(SwapRegionsTest, SwapIntervalsNonConsecutiveNonEdge) {
  std::string R1 = "|region1|", R2 = "; region2;",
              R3 = "---------region3---------", R4 = "++region4++",
              R5 = "***region5***";
  std::string all_regions = R1 + R2 + R3 + R4 + R5;

  // this call should swap R2 with R4.
  SwapIntervals(R1.length(), R1.length() + R2.length() - 1,
                R1.length() + R2.length() + R3.length(),
                R1.length() + R2.length() + R3.length() + R4.length() - 1,
                &all_regions);

  ASSERT_EQ(R1 + R4 + R3 + R2 + R5, all_regions);
}

// Swaps two consecutive regions not in the edge (sorrounded by other regions)
TEST(SwapRegionsTest, SwapIntervalsConsecutiveEdge) {
  std::string R1 = "|region1|", R2 = "; region2;", R3 = "++++region3++++",
              R4 = "---------region4---------";
  std::string all_regions = R1 + R2 + R3 + R4;

  // this call should swap R2 with R3.
  SwapIntervals(R1.length(), R1.length() + R2.length() - 1,
                R1.length() + R2.length(),
                R1.length() + R2.length() + R3.length() - 1, &all_regions);

  ASSERT_EQ(R1 + R3 + R2 + R4, all_regions);
}

// Swaps two consecutive regions not in the edge (not sorrounded by other
// regions)
TEST(SwapRegionsTest, SwapIntervalsConsecutiveNonEdge) {
  std::string R1 = "|region1|", R2 = "; region2;",
              R3 = "---------region3---------", R4 = "++region4++",
              R5 = "***region5***";
  std::string all_regions = R1 + R2 + R3 + R4 + R5;

  // this call should swap R4 with R5.
  SwapIntervals(
      R1.length() + R2.length() + R3.length(),
      R1.length() + R2.length() + R3.length() + R4.length() - 1,
      R1.length() + R2.length() + R3.length() + R4.length(),
      R1.length() + R2.length() + R3.length() + R4.length() + R5.length() - 1,
      &all_regions);

  ASSERT_EQ(R1 + R2 + R3 + R5 + R4, all_regions);
}

}  // namespace
}  // namespace regex_fuzzer
}  // namespace fuzzers
}  // namespace tint
