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

#include "tests/perf_tests/DawnPerfTest.h"

#include "utils/Timer.h"

namespace {

    DawnPerfTestEnvironment* gTestEnv = nullptr;

    constexpr double kMicroSecondsPerSecond = 1e6;
    constexpr double kNanoSecondsPerSecond = 1e9;

}  // namespace

void InitDawnPerfTestEnvironment(int argc, char** argv) {
    gTestEnv = new DawnPerfTestEnvironment(argc, argv);
    DawnTestEnvironment::SetEnvironment(gTestEnv);
    testing::AddGlobalTestEnvironment(gTestEnv);
}

DawnPerfTestEnvironment::DawnPerfTestEnvironment(int argc, char** argv)
    : DawnTestEnvironment(argc, argv) {
    for (int i = 1; i < argc; ++i) {
        if (strcmp("--calibration", argv[i]) == 0) {
            mIsCalibrating = true;
            continue;
        }

        if (strcmp("--override-steps", argv[i]) == 0) {
            const char* value = strchr(argv[i], '=');
            if (value != nullptr) {
                mOverrideStepsToRun = strtoul(value + 1, nullptr, 0);
            }
            continue;
        }

        if (strcmp("-h", argv[i]) == 0 || strcmp("--help", argv[i]) == 0) {
            std::cout << "Additional flags:"
                      << " [--calibration]\n"
                      << "  --calibration: Only run calibration. Calibration allows the perf test"
                         " runner script to save some time.\n"
                      << std::endl;
            continue;
        }
    }
}

DawnPerfTestEnvironment::~DawnPerfTestEnvironment() = default;

void DawnPerfTestEnvironment::SetUp() {
    DawnTestEnvironment::SetUp();
}

bool DawnPerfTestEnvironment::IsCalibrating() const {
    return mIsCalibrating;
}

unsigned int DawnPerfTestEnvironment::OverrideStepsToRun() const {
    return mOverrideStepsToRun;
}

DawnPerfTestBase::DawnPerfTestBase(DawnTestBase* test, unsigned int iterationsPerStep)
    : mTest(test), mIterationsPerStep(iterationsPerStep), mTimer(utils::CreateTimer()) {
}

DawnPerfTestBase::~DawnPerfTestBase() = default;

void DawnPerfTestBase::AbortTest() {
    mRunning = false;
}

void DawnPerfTestBase::WaitForGPU() {
    dawn::FenceDescriptor desc = {};
    desc.initialValue = 0;

    dawn::Fence fence = mTest->queue.CreateFence(&desc);
    mTest->queue.Signal(fence, 1);

    bool done = false;
    fence.OnCompletion(1,
                       [](DawnFenceCompletionStatus status, void* userdata) {
                           ASSERT_EQ(status, DAWN_FENCE_COMPLETION_STATUS_SUCCESS);
                           *reinterpret_cast<bool*>(userdata) = true;
                       },
                       &done);

    while (!done) {
        mTest->WaitABit();
    }
}

void DawnPerfTestBase::RunTest() {
    if (gTestEnv->OverrideStepsToRun() == 0) {
        // Run to compute the approximate number of steps to perform.
        mStepsToRun = std::numeric_limits<unsigned int>::max();

        // Do a warmup run for calibration.
        DoRunLoop(kCalibrationRunTimeSeconds);
        DoRunLoop(kCalibrationRunTimeSeconds);

        // Scale steps down according to the time that exceeded one second.
        double scale = kCalibrationRunTimeSeconds / mTimer->GetElapsedTime();
        mStepsToRun = static_cast<unsigned int>(static_cast<double>(mNumStepsPerformed) * scale);

        // Calibration allows the perf test runner script to save some time.
        if (gTestEnv->IsCalibrating()) {
            PrintResult("steps", mStepsToRun, "count", false);
            return;
        }
    } else {
        mStepsToRun = gTestEnv->OverrideStepsToRun();
    }

    // Do another warmup run. Seems to consistently improve results.
    DoRunLoop(kMaximumRunTimeSeconds);

    for (unsigned int trial = 0; trial < kNumTrials; ++trial) {
        DoRunLoop(kMaximumRunTimeSeconds);
        PrintResults();
    }
}

void DawnPerfTestBase::DoRunLoop(double maxRunTime) {
    mNumStepsPerformed = 0;
    mRunning = true;
    mTimer->Start();

    // This loop can be canceled by calling AbortTest().
    while (mRunning) {
        Step();
        if (mRunning) {
            ++mNumStepsPerformed;
            if (mTimer->GetElapsedTime() > maxRunTime) {
                mRunning = false;
            } else if (mNumStepsPerformed >= mStepsToRun) {
                mRunning = false;
            }
        }
    }

    mTimer->Stop();
}

void DawnPerfTestBase::PrintResults() {
    double elapsedTimeSeconds[2] = {
        mTimer->GetElapsedTime(),
        mGPUTimeNs * 1e-9,
    };

    const char* clockNames[2] = {
        "wall_time",
        "gpu_time",
    };

    // If measured gpu time is non-zero, print that too.
    unsigned int clocksToOutput = mGPUTimeNs > 0 ? 2 : 1;

    for (unsigned int i = 0; i < clocksToOutput; ++i) {
        double secondsPerStep = elapsedTimeSeconds[i] / static_cast<double>(mNumStepsPerformed);
        double secondsPerIteration = secondsPerStep / static_cast<double>(mIterationsPerStep);

        // Give the result a different name to ensure separate graphs if we transition.
        if (secondsPerIteration > 1e-3) {
            double microSecondsPerIteration = secondsPerIteration * kMicroSecondsPerSecond;
            PrintResult(clockNames[i], microSecondsPerIteration, "us", true);
        } else {
            double nanoSecPerIteration = secondsPerIteration * kNanoSecondsPerSecond;
            PrintResult(clockNames[i], nanoSecPerIteration, "ns", true);
        }
    }
}

void DawnPerfTestBase::PrintResult(const std::string& trace,
                                   double value,
                                   const std::string& units,
                                   bool important) const {
    const ::testing::TestInfo* const testInfo =
        ::testing::UnitTest::GetInstance()->current_test_info();

    const char* testName = testInfo->name();
    const char* testSuite = testInfo->test_suite_name();

    // The results are printed according to the format specified at
    // [chromium]//build/scripts/slave/performance_log_processor.py
    fflush(stdout);
    printf("%sRESULT %s%s: %s= %s%f%s %s\n", important ? "*" : "", testSuite, testName,
           trace.c_str(), "", value, "", units.c_str());
    fflush(stdout);
}

void DawnPerfTestBase::PrintResult(const std::string& trace,
                                   unsigned int value,
                                   const std::string& units,
                                   bool important) const {
    const ::testing::TestInfo* const testInfo =
        ::testing::UnitTest::GetInstance()->current_test_info();

    const char* testName = testInfo->name();
    const char* testSuite = testInfo->test_suite_name();

    // The results are printed according to the format specified at
    // [chromium]//build/scripts/slave/performance_log_processor.py
    fflush(stdout);
    printf("%sRESULT %s%s: %s= %s%u%s %s\n", important ? "*" : "", testName, testSuite,
           trace.c_str(), "", value, "", units.c_str());
    fflush(stdout);
}
