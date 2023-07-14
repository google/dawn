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

#include <memory>

#include "dawn/platform/metrics/HistogramMacros.h"
#include "dawn/tests/DawnTest.h"

namespace dawn {
namespace {

using ::testing::_;
using ::testing::InSequence;
using ::testing::NiceMock;

class DawnHistogramMockPlatform : public dawn::platform::Platform {
  public:
    void SetTime(double time) { mTime = time; }

    double MonotonicallyIncreasingTime() override { return mTime; }

    MOCK_METHOD(void,
                HistogramCustomCounts,
                (const char* name, int sample, int min, int max, int bucketCount),
                (override));

    MOCK_METHOD(void,
                HistogramCustomCountsHPC,
                (const char* name, int sample, int min, int max, int bucketCount),
                (override));

    MOCK_METHOD(void,
                HistogramEnumeration,
                (const char* name, int sample, int boundaryValue),
                (override));

    MOCK_METHOD(void, HistogramSparse, (const char* name, int sample), (override));

    MOCK_METHOD(void, HistogramBoolean, (const char* name, bool sample), (override));

  private:
    double mTime = 0.0;
};

class HistogramTests : public DawnTest {
  protected:
    void SetUp() override { DawnTest::SetUp(); }

    std::unique_ptr<platform::Platform> CreateTestPlatform() override {
        auto p = std::make_unique<NiceMock<DawnHistogramMockPlatform>>();
        mMockPlatform = p.get();
        return p;
    }

    NiceMock<DawnHistogramMockPlatform>* mMockPlatform;
};

TEST_P(HistogramTests, Times) {
    InSequence seq;
    EXPECT_CALL(*mMockPlatform, HistogramCustomCounts("times", 1, _, _, _));
    EXPECT_CALL(*mMockPlatform, HistogramCustomCounts("medium_times", 2, _, _, _));
    EXPECT_CALL(*mMockPlatform, HistogramCustomCounts("long_times", 3, _, _, _));
    EXPECT_CALL(*mMockPlatform, HistogramCustomCounts("long_times_100", 4, _, _, _));
    EXPECT_CALL(*mMockPlatform, HistogramCustomCounts("custom_times", 5, 0, 10, 42));
    EXPECT_CALL(*mMockPlatform,
                HistogramCustomCountsHPC("custom_microsecond_times", 6, 0, 100, 420));

    DAWN_HISTOGRAM_TIMES(mMockPlatform, "times", 1);
    DAWN_HISTOGRAM_MEDIUM_TIMES(mMockPlatform, "medium_times", 2);
    DAWN_HISTOGRAM_LONG_TIMES(mMockPlatform, "long_times", 3);
    DAWN_HISTOGRAM_LONG_TIMES_100(mMockPlatform, "long_times_100", 4);
    DAWN_HISTOGRAM_CUSTOM_TIMES(mMockPlatform, "custom_times", 5, 0, 10, 42);
    DAWN_HISTOGRAM_CUSTOM_MICROSECOND_TIMES(mMockPlatform, "custom_microsecond_times", 6, 0, 100,
                                            420);
}

TEST_P(HistogramTests, Percentage) {
    InSequence seq;
    EXPECT_CALL(*mMockPlatform, HistogramEnumeration("percentage", 0, 101));
    EXPECT_CALL(*mMockPlatform, HistogramEnumeration("percentage", 42, 101));
    EXPECT_CALL(*mMockPlatform, HistogramEnumeration("percentage", 100, 101));

    DAWN_HISTOGRAM_PERCENTAGE(mMockPlatform, "percentage", 0);
    DAWN_HISTOGRAM_PERCENTAGE(mMockPlatform, "percentage", 42);
    DAWN_HISTOGRAM_PERCENTAGE(mMockPlatform, "percentage", 100);
}

TEST_P(HistogramTests, Boolean) {
    InSequence seq;
    EXPECT_CALL(*mMockPlatform, HistogramBoolean("boolean", false));
    EXPECT_CALL(*mMockPlatform, HistogramBoolean("boolean", true));

    DAWN_HISTOGRAM_BOOLEAN(mMockPlatform, "boolean", false);
    DAWN_HISTOGRAM_BOOLEAN(mMockPlatform, "boolean", true);
}

TEST_P(HistogramTests, Enumeration) {
    enum Animal { Dog, Cat, Bear, Count };

    InSequence seq;
    EXPECT_CALL(*mMockPlatform, HistogramEnumeration("animal", Animal::Dog, Animal::Count));
    EXPECT_CALL(*mMockPlatform, HistogramEnumeration("animal", Animal::Cat, Animal::Count));
    EXPECT_CALL(*mMockPlatform, HistogramEnumeration("animal", Animal::Bear, Animal::Count));

    DAWN_HISTOGRAM_ENUMERATION(mMockPlatform, "animal", Animal::Dog, Animal::Count);
    DAWN_HISTOGRAM_ENUMERATION(mMockPlatform, "animal", Animal::Cat, Animal::Count);
    DAWN_HISTOGRAM_ENUMERATION(mMockPlatform, "animal", Animal::Bear, Animal::Count);
}

TEST_P(HistogramTests, Memory) {
    InSequence seq;
    EXPECT_CALL(*mMockPlatform, HistogramCustomCounts("kb", 1, _, _, _));
    EXPECT_CALL(*mMockPlatform, HistogramCustomCounts("mb", 2, _, _, _));

    DAWN_HISTOGRAM_MEMORY_KB(mMockPlatform, "kb", 1);
    DAWN_HISTOGRAM_MEMORY_MB(mMockPlatform, "mb", 2);
}

TEST_P(HistogramTests, Sparse) {
    InSequence seq;
    EXPECT_CALL(*mMockPlatform, HistogramSparse("sparse", 1));
    EXPECT_CALL(*mMockPlatform, HistogramSparse("sparse", 2));

    DAWN_HISTOGRAM_SPARSE(mMockPlatform, "sparse", 1);
    DAWN_HISTOGRAM_SPARSE(mMockPlatform, "sparse", 2);
}

TEST_P(HistogramTests, ScopedTimer) {
    InSequence seq;
    EXPECT_CALL(*mMockPlatform, HistogramCustomCounts("timer0", 2'500, _, _, _));
    EXPECT_CALL(*mMockPlatform, HistogramCustomCounts("timer1", 15'500, _, _, _));
    EXPECT_CALL(*mMockPlatform, HistogramCustomCounts("timer4", 2'500, _, _, _));
    EXPECT_CALL(*mMockPlatform, HistogramCustomCounts("timer3", 6'000, _, _, _));
    EXPECT_CALL(*mMockPlatform, HistogramCustomCountsHPC("timer5", 1'500'000, _, _, _));

    {
        mMockPlatform->SetTime(1.0);
        SCOPED_DAWN_HISTOGRAM_TIMER(mMockPlatform, "timer0");
        mMockPlatform->SetTime(3.5);
    }
    {
        mMockPlatform->SetTime(10.0);
        SCOPED_DAWN_HISTOGRAM_LONG_TIMER(mMockPlatform, "timer1");
        mMockPlatform->SetTime(25.5);
    }
    {
        mMockPlatform->SetTime(1.0);
        SCOPED_DAWN_HISTOGRAM_TIMER(mMockPlatform, "timer3");
        mMockPlatform->SetTime(4.5);
        SCOPED_DAWN_HISTOGRAM_LONG_TIMER(mMockPlatform, "timer4");
        mMockPlatform->SetTime(7.0);
    }
    {
        mMockPlatform->SetTime(1.0);
        SCOPED_DAWN_HISTOGRAM_TIMER_MICROS(mMockPlatform, "timer5");
        mMockPlatform->SetTime(2.5);
    }
}

DAWN_INSTANTIATE_TEST(HistogramTests,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      NullBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

}  // namespace
}  // namespace dawn
