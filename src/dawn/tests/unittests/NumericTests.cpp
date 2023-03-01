// Copyright 2023 The Dawn Authors
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

#include "dawn/common/Numeric.h"
#include "gtest/gtest.h"

// Tests for RangesOverlap
TEST(Numeric, RangesOverlap) {
    // Range contains only one number
    ASSERT_EQ(true, RangesOverlap(0, 0, 0, 0));
    ASSERT_EQ(false, RangesOverlap(0, 0, 1, 1));

    // [  ]
    //      [  ]
    ASSERT_EQ(false, RangesOverlap(0, 8, 9, 16));

    //      [  ]
    // [  ]
    ASSERT_EQ(false, RangesOverlap(9, 16, 0, 8));

    //      [  ]
    // [             ]
    ASSERT_EQ(true, RangesOverlap(2, 3, 0, 8));

    // [             ]
    //      [  ]
    ASSERT_EQ(true, RangesOverlap(0, 8, 2, 3));

    // [   ]
    //   [   ]
    ASSERT_EQ(true, RangesOverlap(0, 8, 4, 12));

    //   [   ]
    // [   ]
    ASSERT_EQ(true, RangesOverlap(4, 12, 0, 8));

    // [   ]
    //     [   ]
    ASSERT_EQ(true, RangesOverlap(0, 8, 8, 12));

    //     [   ]
    // [   ]
    ASSERT_EQ(true, RangesOverlap(8, 12, 0, 8));

    // Negative numbers
    ASSERT_EQ(true, RangesOverlap(-9, 12, 4, 16));
    ASSERT_EQ(false, RangesOverlap(-9, -3, -2, 0));
}
