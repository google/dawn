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

#include "dawn_platform/tracing/TraceEvent.h"
#include "tests/perf_tests/DawnPerfTestPlatform.h"
#include "utils/Timer.h"

#include <json/value.h>
#include <json/writer.h>
#include <fstream>

namespace {

    DawnPerfTestEnvironment* gTestEnv = nullptr;

    constexpr double kMicroSecondsPerSecond = 1e6;
    constexpr double kNanoSecondsPerSecond = 1e9;

    void DumpTraceEventsToJSONFile(
        const std::vector<DawnPerfTestPlatform::TraceEvent>& traceEventBuffer,
        const char* traceFile) {
        std::ofstream outFile;
        outFile.open(traceFile, std::ios_base::app);

        Json::StreamWriterBuilder builder;
        std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());

        for (const DawnPerfTestPlatform::TraceEvent& traceEvent : traceEventBuffer) {
            Json::Value value(Json::objectValue);

            const Json::LargestInt microseconds =
                static_cast<Json::LargestInt>(traceEvent.timestamp * 1000.0 * 1000.0);

            char phase[2] = {traceEvent.phase, '\0'};

            value["name"] = traceEvent.name;
            value["cat"] = traceEvent.categoryName;
            value["ph"] = &phase[0];
            value["id"] = traceEvent.id;
            value["ts"] = microseconds;
            value["pid"] = "Dawn";

            outFile << ", ";
            writer->write(value, &outFile);
            outFile.flush();
        }

        outFile.close();
    }

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

        constexpr const char kOverrideStepsArg[] = "--override-steps=";
        if (strstr(argv[i], kOverrideStepsArg) == argv[i]) {
            const char* overrideSteps = argv[i] + strlen(kOverrideStepsArg);
            if (overrideSteps[0] != '\0') {
                mOverrideStepsToRun = strtoul(overrideSteps, nullptr, 0);
            }
            continue;
        }

        constexpr const char kTraceFileArg[] = "--trace-file=";
        if (strstr(argv[i], kTraceFileArg) == argv[i]) {
            const char* traceFile = argv[i] + strlen(kTraceFileArg);
            if (traceFile[0] != '\0') {
                mTraceFile = traceFile;
            }
            continue;
        }

        if (strcmp("-h", argv[i]) == 0 || strcmp("--help", argv[i]) == 0) {
            std::cout
                << "Additional flags:"
                << " [--calibration] [--override-steps=x] [--enable-tracing] [--trace-file=file]\n"
                << "  --calibration: Only run calibration. Calibration allows the perf test"
                   " runner script to save some time.\n"
                << " --override-steps: Set a fixed number of steps to run for each test\n"
                << " --enable-tracing: Enable tracing of Dawn's internals.\n"
                << " --trace-file: The file to dump trace results.\n"
                << std::endl;
            continue;
        }
    }
}

DawnPerfTestEnvironment::~DawnPerfTestEnvironment() = default;

void DawnPerfTestEnvironment::SetUp() {
    DawnTestEnvironment::SetUp();

    mPlatform = std::make_unique<DawnPerfTestPlatform>();
    mInstance->SetPlatform(mPlatform.get());

    // Begin writing the trace event array.
    if (mTraceFile != nullptr) {
        std::ofstream outFile;
        outFile.open(mTraceFile);
        outFile << "{ \"traceEvents\": [";
        outFile << "{}";  // Dummy object so trace events can always prepend a comma
        outFile.flush();
        outFile.close();
    }
}

void DawnPerfTestEnvironment::TearDown() {
    // End writing the trace event array.
    if (mTraceFile != nullptr) {
        std::vector<DawnPerfTestPlatform::TraceEvent> traceEventBuffer =
            mPlatform->AcquireTraceEventBuffer();

        // Write remaining trace events.
        DumpTraceEventsToJSONFile(traceEventBuffer, mTraceFile);

        std::ofstream outFile;
        outFile.open(mTraceFile, std::ios_base::app);
        outFile << "]}";
        outFile << std::endl;
        outFile.close();
    }

    DawnTestEnvironment::TearDown();
}

bool DawnPerfTestEnvironment::IsCalibrating() const {
    return mIsCalibrating;
}

unsigned int DawnPerfTestEnvironment::OverrideStepsToRun() const {
    return mOverrideStepsToRun;
}

const char* DawnPerfTestEnvironment::GetTraceFile() const {
    return mTraceFile;
}

DawnPerfTestBase::DawnPerfTestBase(DawnTestBase* test,
                                   unsigned int iterationsPerStep,
                                   unsigned int maxStepsInFlight)
    : mTest(test),
      mIterationsPerStep(iterationsPerStep),
      mMaxStepsInFlight(maxStepsInFlight),
      mTimer(utils::CreateTimer()) {
}

DawnPerfTestBase::~DawnPerfTestBase() = default;

void DawnPerfTestBase::AbortTest() {
    mRunning = false;
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

    DawnPerfTestPlatform* platform =
        reinterpret_cast<DawnPerfTestPlatform*>(gTestEnv->GetInstance()->GetPlatform());
    const char* testName = ::testing::UnitTest::GetInstance()->current_test_info()->name();

    // Only enable trace event recording in this section.
    // We don't care about trace events during warmup and calibration.
    platform->EnableTraceEventRecording(true);
    {
        TRACE_EVENT0(platform, "dawn.perf_test", testName);
        for (unsigned int trial = 0; trial < kNumTrials; ++trial) {
            TRACE_EVENT0(platform, "dawn.perf_test", "Trial");
            DoRunLoop(kMaximumRunTimeSeconds);
            OutputResults();
        }
    }
    platform->EnableTraceEventRecording(false);
}

void DawnPerfTestBase::DoRunLoop(double maxRunTime) {
    dawn_platform::Platform* platform = gTestEnv->GetInstance()->GetPlatform();

    mNumStepsPerformed = 0;
    mRunning = true;

    wgpu::FenceDescriptor desc = {};
    uint64_t signaledFenceValue = 0;
    wgpu::Fence fence = mTest->queue.CreateFence(&desc);

    mTimer->Start();

    // This loop can be canceled by calling AbortTest().
    while (mRunning) {
        // Wait if there are too many steps in flight on the GPU.
        while (signaledFenceValue - fence.GetCompletedValue() >= mMaxStepsInFlight) {
            mTest->WaitABit();
        }
        TRACE_EVENT0(platform, "dawn.perf_test", "Step");
        Step();
        mTest->queue.Signal(fence, ++signaledFenceValue);

        if (mRunning) {
            ++mNumStepsPerformed;
            if (mTimer->GetElapsedTime() > maxRunTime) {
                mRunning = false;
            } else if (mNumStepsPerformed >= mStepsToRun) {
                mRunning = false;
            }
        }
    }

    // Wait for all GPU commands to complete.
    while (signaledFenceValue != fence.GetCompletedValue()) {
        mTest->WaitABit();
    }

    mTimer->Stop();
}

void DawnPerfTestBase::OutputResults() {
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

    DawnPerfTestPlatform* platform =
        reinterpret_cast<DawnPerfTestPlatform*>(gTestEnv->GetInstance()->GetPlatform());

    std::vector<DawnPerfTestPlatform::TraceEvent> traceEventBuffer =
        platform->AcquireTraceEventBuffer();

    // TODO(enga): Process traces to extract time of command recording, validation, etc.

    const char* traceFile = gTestEnv->GetTraceFile();
    if (traceFile != nullptr) {
        DumpTraceEventsToJSONFile(traceEventBuffer, traceFile);
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
