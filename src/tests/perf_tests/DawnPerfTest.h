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

#ifndef TESTS_PERFTESTS_DAWNPERFTEST_H_
#define TESTS_PERFTESTS_DAWNPERFTEST_H_

#include "tests/DawnTest.h"

namespace utils {
    class Timer;
}

void InitDawnPerfTestEnvironment(int argc, char** argv);

class DawnPerfTestEnvironment : public DawnTestEnvironment {
  public:
    DawnPerfTestEnvironment(int argc, char** argv);
    ~DawnPerfTestEnvironment();

    void SetUp() override;

    bool IsCalibrating() const;
    unsigned int OverrideStepsToRun() const;

  private:
    // Only run calibration which allows the perf test runner to save time.
    bool mIsCalibrating = false;

    // If non-zero, overrides the number of steps.
    unsigned int mOverrideStepsToRun = 0;
};

// Dawn Perf Tests calls Step() of a derived class to measure its execution
// time. First, a calibration step is run which determines the number of times
// to call Step() to last approximately |kCalibrationRunTimeSeconds|. Then,
// Step() is called for the computed number of times, or until
// |kMaximumRunTimeSeconds| is exceeded. |kNumTrials| are performed and the
// results and averages per iteration** are printed.
//
// The results are printed according to the format specified at
// [chromium]//build/scripts/slave/performance_log_processor.py
//
// ** The number of iterations a test performs should be passed to the
// constructor of DawnPerfTestBase. The reported times are the total time
// divided by (numSteps * iterationsPerStep).
class DawnPerfTestBase {
    static constexpr double kCalibrationRunTimeSeconds = 1.0;
    static constexpr double kMaximumRunTimeSeconds = 10.0;
    static constexpr unsigned int kNumTrials = 3;

  public:
    DawnPerfTestBase(DawnTestBase* test, unsigned int iterationsPerStep);
    virtual ~DawnPerfTestBase();

  protected:
    // Call if the test step was aborted and the test should stop running.
    void AbortTest();

    void WaitForGPU();

    void RunTest();
    void PrintResult(const std::string& trace,
                     double value,
                     const std::string& units,
                     bool important) const;
    void PrintResult(const std::string& trace,
                     unsigned int value,
                     const std::string& units,
                     bool important) const;

  private:
    void DoRunLoop(double maxRunTime);
    void PrintResults();

    virtual void Step() = 0;

    DawnTestBase* mTest;
    bool mRunning = false;
    unsigned int mIterationsPerStep;
    unsigned int mStepsToRun = 0;
    unsigned int mNumStepsPerformed = 0;
    uint64_t mGPUTimeNs = 0;  // TODO(enga): Measure GPU time with timing queries.
    std::unique_ptr<utils::Timer> mTimer;
};

template <typename Params = DawnTestParam>
class DawnPerfTestWithParams : public DawnTestWithParams<Params>, public DawnPerfTestBase {
  protected:
    DawnPerfTestWithParams(unsigned int iterationsPerStep)
        : DawnTestWithParams<Params>(), DawnPerfTestBase(this, iterationsPerStep) {
    }
    ~DawnPerfTestWithParams() override = default;
};

using DawnPerfTest = DawnPerfTestWithParams<>;

#define DAWN_INSTANTIATE_PERF_TEST_SUITE_P(testName, ...)                                      \
    INSTANTIATE_TEST_SUITE_P(                                                                  \
        , testName, ::testing::ValuesIn(MakeParamGenerator<testName::ParamType>(__VA_ARGS__)), \
        testing::PrintToStringParamName())

#endif  // TESTS_PERFTESTS_DAWNPERFTEST_H_
