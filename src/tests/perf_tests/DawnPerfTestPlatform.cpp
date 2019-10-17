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

#include "dawn_platform/tracing/TraceEvent.h"
#include "tests/perf_tests/DawnPerfTest.h"
#include "utils/Timer.h"

namespace {

    struct TraceCategory {
        unsigned char enabled;
        const char* name;
    };

    constexpr TraceCategory gTraceCategories[2] = {
        // TODO(enga): Remove the use of this macro, but keep it disabled by default in Chromium.
        {1, TRACE_DISABLED_BY_DEFAULT("gpu.dawn")},
        {1, "dawn.perf_test"},
    };

}  // anonymous namespace

DawnPerfTestPlatform::DawnPerfTestPlatform(bool enableTracing)
    : dawn_platform::Platform(), mEnableTracing(enableTracing), mTimer(utils::CreateTimer()) {
}

DawnPerfTestPlatform::~DawnPerfTestPlatform() = default;

const unsigned char* DawnPerfTestPlatform::GetTraceCategoryEnabledFlag(const char* name) {
    if (mEnableTracing) {
        for (const TraceCategory& category : gTraceCategories) {
            if (strcmp(category.name, name) == 0) {
                return &category.enabled;
            }
        }
    }

    constexpr static unsigned char kZero = 0;
    return &kZero;
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
    if (!mEnableTracing || !mRecordTraceEvents) {
        return 0;
    }

    // Discover the category name based on categoryGroupEnabled.  This flag comes from the first
    // parameter of TraceCategory, and corresponds to one of the entries in gTraceCategories.
    static_assert(offsetof(TraceCategory, enabled) == 0,
                  "|enabled| must be the first field of the TraceCategory class.");
    const TraceCategory* category = reinterpret_cast<const TraceCategory*>(categoryGroupEnabled);

    mTraceEventBuffer.emplace_back(phase, category->name, name, timestamp);
    return static_cast<uint64_t>(mTraceEventBuffer.size());
}

void DawnPerfTestPlatform::EnableTraceEventRecording(bool enable) {
    mRecordTraceEvents = enable;
}

const std::vector<DawnPerfTestPlatform::TraceEvent>& DawnPerfTestPlatform::GetTraceEventBuffer()
    const {
    return mTraceEventBuffer;
}
