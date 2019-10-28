// Copyright 2019 The Dawn Authors
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

#include "tests/perf_tests/DawnPerfTestPlatform.h"

#include "common/Assert.h"
#include "dawn_platform/tracing/TraceEvent.h"
#include "tests/perf_tests/DawnPerfTest.h"
#include "utils/Timer.h"

namespace {

    struct TraceCategoryInfo {
        unsigned char enabled;
        dawn_platform::TraceCategory category;
    };

    constexpr TraceCategoryInfo gTraceCategories[4] = {
        {1, dawn_platform::TraceCategory::General},
        {1, dawn_platform::TraceCategory::Validation},
        {1, dawn_platform::TraceCategory::Recording},
        {1, dawn_platform::TraceCategory::GPUWork},
    };

    static_assert(static_cast<uint32_t>(dawn_platform::TraceCategory::General) == 0, "");
    static_assert(static_cast<uint32_t>(dawn_platform::TraceCategory::Validation) == 1, "");
    static_assert(static_cast<uint32_t>(dawn_platform::TraceCategory::Recording) == 2, "");
    static_assert(static_cast<uint32_t>(dawn_platform::TraceCategory::GPUWork) == 3, "");

}  // anonymous namespace

DawnPerfTestPlatform::DawnPerfTestPlatform()
    : dawn_platform::Platform(), mTimer(utils::CreateTimer()) {
}

DawnPerfTestPlatform::~DawnPerfTestPlatform() = default;

const unsigned char* DawnPerfTestPlatform::GetTraceCategoryEnabledFlag(
    dawn_platform::TraceCategory category) {
    switch (category) {
        case dawn_platform::TraceCategory::General:
        case dawn_platform::TraceCategory::Validation:
        case dawn_platform::TraceCategory::Recording:
        case dawn_platform::TraceCategory::GPUWork:
            break;
        default:
            UNREACHABLE();
    }
    return &gTraceCategories[static_cast<uint32_t>(category)].enabled;
}

double DawnPerfTestPlatform::MonotonicallyIncreasingTime() {
    // Move the time origin to the first call to this function, to avoid generating
    // unnecessarily large timestamps.
    static double origin = mTimer->GetAbsoluteTime();
    return mTimer->GetAbsoluteTime() - origin;
}

// TODO(enga): Simplify this API.
uint64_t DawnPerfTestPlatform::AddTraceEvent(char phase,
                                             const unsigned char* categoryGroupEnabled,
                                             const char* name,
                                             uint64_t id,
                                             double timestamp,
                                             int numArgs,
                                             const char** argNames,
                                             const unsigned char* argTypes,
                                             const uint64_t* argValues,
                                             unsigned char flags) {
    if (!mRecordTraceEvents) {
        return 0;
    }

    // Discover the category name based on categoryGroupEnabled.  This flag comes from the first
    // parameter of TraceCategory, and corresponds to one of the entries in gTraceCategories.
    static_assert(offsetof(TraceCategoryInfo, enabled) == 0,
                  "|enabled| must be the first field of the TraceCategoryInfo class.");

    const TraceCategoryInfo* info =
        reinterpret_cast<const TraceCategoryInfo*>(categoryGroupEnabled);

    mTraceEventBuffer.emplace_back(phase, info->category, name, id, timestamp);
    return static_cast<uint64_t>(mTraceEventBuffer.size());
}

void DawnPerfTestPlatform::EnableTraceEventRecording(bool enable) {
    mRecordTraceEvents = enable;
}

std::vector<DawnPerfTestPlatform::TraceEvent> DawnPerfTestPlatform::AcquireTraceEventBuffer() {
    std::vector<DawnPerfTestPlatform::TraceEvent> buffer = mTraceEventBuffer;
    mTraceEventBuffer.clear();
    return buffer;
}
