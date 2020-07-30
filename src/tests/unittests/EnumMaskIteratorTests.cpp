// Copyright 2020 The Dawn Authors
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

#include "dawn_native/EnumMaskIterator.h"

#include "gtest/gtest.h"

namespace dawn_native {

    enum class TestAspect : uint8_t {
        Color = 1,
        Depth = 2,
        Stencil = 4,
    };

    template <>
    struct EnumBitmaskSize<TestAspect> {
        static constexpr unsigned value = 3;
    };

}  // namespace dawn_native

namespace wgpu {

    template <>
    struct IsDawnBitmask<dawn_native::TestAspect> {
        static constexpr bool enable = true;
    };

}  // namespace wgpu

namespace dawn_native {

    static_assert(EnumBitmaskSize<TestAspect>::value == 3, "");

    TEST(EnumMaskIteratorTests, None) {
        for (TestAspect aspect : IterateEnumMask(static_cast<TestAspect>(0))) {
            FAIL();
            DAWN_UNUSED(aspect);
        }
    }

    TEST(EnumMaskIteratorTests, All) {
        TestAspect expected[] = {TestAspect::Color, TestAspect::Depth, TestAspect::Stencil};
        uint32_t i = 0;
        TestAspect aspects = TestAspect::Color | TestAspect::Depth | TestAspect::Stencil;
        for (TestAspect aspect : IterateEnumMask(aspects)) {
            EXPECT_EQ(aspect, expected[i++]);
        }
    }

    TEST(EnumMaskIteratorTests, Partial) {
        TestAspect expected[] = {TestAspect::Color, TestAspect::Stencil};
        uint32_t i = 0;
        TestAspect aspects = TestAspect::Stencil | TestAspect::Color;
        for (TestAspect aspect : IterateEnumMask(aspects)) {
            EXPECT_EQ(aspect, expected[i++]);
        }
    }

}  // namespace dawn_native
