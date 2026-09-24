// Copyright 2026 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"

#include <gtest/gtest.h>

#include <chrono>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "src/dawn/node/standalone/EventLoop.h"
#include "src/dawn/node/test/V8TestEnvironment.h"

namespace dawn::node::standalone {

namespace {

using IterationResult = EventLoop::IterationResult;
using std::chrono::milliseconds;

// An EventLoop whose clock the test moves by hand, so that delays cost no wall time and the order
// tasks run in is fully determined rather than merely likely. Run() reads the real clock when it
// sleeps and so cannot be used with this; the tests that cover Run() use a plain EventLoop.
class TestEventLoop : public EventLoop {
  public:
    using EventLoop::EventLoop;

    // Moves the clock forward without running anything.
    void AdvanceBy(Duration delta) { now_ += delta; }

  protected:
    TimePoint Now() const override { return now_; }

  private:
    // Start away from the epoch so that a time computed from a negative delay is still orderable.
    TimePoint now_ = TimePoint() + std::chrono::hours(1);
};

// Redirects std::cerr into a buffer for its lifetime, so that a test that expects the loop to
// report an uncaught exception can assert on the report rather than printing it.
class StderrCapture {
  public:
    StderrCapture() : original_(std::cerr.rdbuf(buffer_.rdbuf())) {}
    ~StderrCapture() { std::cerr.rdbuf(original_); }

    std::string str() const { return buffer_.str(); }

  private:
    std::ostringstream buffer_;
    std::streambuf* const original_;
};

// Renders the log of what ran as one string, so that a failure prints the whole order rather than
// the first element that differs.
std::string Join(const std::vector<std::string>& entries) {
    std::string joined;
    for (const std::string& entry : entries) {
        if (!joined.empty()) {
            joined += ",";
        }
        joined += entry;
    }
    return joined;
}

// Posts a task that posts itself again every time it runs, the way Dawn's AsyncRunner polls for
// completed work for as long as any is outstanding.
void PostSelfRePostingTask(EventLoop* loop, int* runs) {
    loop->PostTask([loop, runs] {
        ++*runs;
        PostSelfRePostingTask(loop, runs);
    });
}

// The loop runs its tasks inside a handle scope and drains microtasks between them, so it needs an
// entered isolate with an entered context even for the tests whose tasks are pure C++. Microtasks
// are explicit so that the checkpoint the loop performs between tasks is the only thing that
// drains them and a test can pin down when they run.
class EventLoopTest : public test::V8IsolateTest {
  protected:
    EventLoopTest() : V8IsolateTest(v8::MicrotasksPolicy::kExplicit) {}

    // Installs a global record(name) that appends to `log`. The tasks in these tests are written
    // in C++ and the microtasks in JavaScript, so they need one shared list to be ordered against
    // each other.
    void InstallRecorder(std::vector<std::string>* log) {
        v8::Local<v8::External> data =
            v8::External::New(isolate_, log, v8::kExternalPointerTypeTagDefault);
        v8::Local<v8::Function> record = v8::FunctionTemplate::New(isolate_, Record, data)
                                             ->GetFunction(context())
                                             .ToLocalChecked();
        context()
            ->Global()
            ->Set(context(), v8::String::NewFromUtf8Literal(isolate_, "record"), record)
            .Check();
    }

    // Evaluates `source` in the entered context. A script that throws leaves the exception pending
    // rather than reporting it here, which is what lets a test hand one to the loop.
    void RunScript(const char* source) {
        v8::Local<v8::String> text = v8::String::NewFromUtf8(isolate_, source).ToLocalChecked();
        v8::Local<v8::Script> script;
        if (!v8::Script::Compile(context(), text).ToLocal(&script)) {
            return;
        }
        static_cast<void>(script->Run(context()));
    }

    v8::Platform* const platform_ = test::V8Platform();

  private:
    static void Record(const v8::FunctionCallbackInfo<v8::Value>& info) {
        auto* log = static_cast<std::vector<std::string>*>(
            info.Data().As<v8::External>()->Value(v8::kExternalPointerTypeTagDefault));
        v8::String::Utf8Value name(info.GetIsolate(), info[0]);
        log->emplace_back(*name != nullptr ? *name : "");
    }
};

TEST_F(EventLoopTest, ImmediatesRunInTheOrderTheyWerePosted) {
    TestEventLoop loop(isolate_, platform_);
    std::vector<std::string> order;

    loop.PostTask([&] { order.push_back("first"); });
    loop.PostTask([&] { order.push_back("second"); });
    loop.PostTask([&] { order.push_back("third"); });

    EXPECT_EQ(loop.RunOneIteration(), IterationResult::kRanTasks);
    EXPECT_EQ(Join(order), "first,second,third");
}

TEST_F(EventLoopTest, TimersRunInDueOrder) {
    TestEventLoop loop(isolate_, platform_);
    std::vector<std::string> order;

    loop.PostDelayedTask([&] { order.push_back("30ms"); }, milliseconds(30));
    loop.PostDelayedTask([&] { order.push_back("10ms"); }, milliseconds(10));
    loop.PostDelayedTask([&] { order.push_back("20ms"); }, milliseconds(20));

    loop.AdvanceBy(milliseconds(30));
    EXPECT_EQ(loop.RunOneIteration(), IterationResult::kRanTasks);
    EXPECT_EQ(Join(order), "10ms,20ms,30ms");
}

TEST_F(EventLoopTest, TimersWithTheSameDeadlineRunInTheOrderTheyWerePosted) {
    TestEventLoop loop(isolate_, platform_);
    std::vector<std::string> order;

    loop.PostDelayedTask([&] { order.push_back("first"); }, milliseconds(10));
    loop.PostDelayedTask([&] { order.push_back("second"); }, milliseconds(10));
    loop.PostDelayedTask([&] { order.push_back("third"); }, milliseconds(10));

    loop.AdvanceBy(milliseconds(10));
    EXPECT_EQ(loop.RunOneIteration(), IterationResult::kRanTasks);
    EXPECT_EQ(Join(order), "first,second,third");
}

TEST_F(EventLoopTest, ATimerThatIsNotYetDueDoesNotRun) {
    TestEventLoop loop(isolate_, platform_);
    bool ran = false;

    loop.PostDelayedTask([&] { ran = true; }, milliseconds(10));

    loop.AdvanceBy(milliseconds(9));
    EXPECT_EQ(loop.RunOneIteration(), IterationResult::kIdle);
    EXPECT_FALSE(ran);

    loop.AdvanceBy(milliseconds(1));
    EXPECT_EQ(loop.RunOneIteration(), IterationResult::kRanTasks);
    EXPECT_TRUE(ran);
}

TEST_F(EventLoopTest, ATimerArmedFromInsideTheTimersPhaseWaitsForTheNextIteration) {
    TestEventLoop loop(isolate_, platform_);
    std::vector<std::string> order;

    loop.PostDelayedTask(
        [&] {
            order.push_back("outer");
            loop.PostDelayedTask([&] { order.push_back("inner"); }, milliseconds(0));
        },
        milliseconds(0));

    // The clock does not move while the phase runs, so the inner timer comes due at exactly the
    // phase cutoff. It must still be held over, or a repeating zero-delay timer would keep the
    // loop in the timers phase indefinitely.
    EXPECT_EQ(loop.RunOneIteration(), IterationResult::kRanTasks);
    EXPECT_EQ(Join(order), "outer");

    order.clear();
    EXPECT_EQ(loop.RunOneIteration(), IterationResult::kRanTasks);
    EXPECT_EQ(Join(order), "inner");
}

TEST_F(EventLoopTest, AnImmediatePostedFromInsideTheCheckPhaseWaitsForTheNextIteration) {
    TestEventLoop loop(isolate_, platform_);
    std::vector<std::string> order;

    loop.PostTask([&] {
        order.push_back("outer");
        loop.PostTask([&] { order.push_back("inner"); });
    });

    EXPECT_EQ(loop.RunOneIteration(), IterationResult::kRanTasks);
    EXPECT_EQ(Join(order), "outer");

    EXPECT_EQ(loop.RunOneIteration(), IterationResult::kRanTasks);
    EXPECT_EQ(Join(order), "outer,inner");
}

TEST_F(EventLoopTest, ASelfRePostingImmediateDoesNotStarveTimers) {
    TestEventLoop loop(isolate_, platform_);
    int polls = 0;
    bool timer_ran = false;

    PostSelfRePostingTask(&loop, &polls);
    loop.PostDelayedTask([&] { timer_ran = true; }, milliseconds(10));

    // The poller runs once per iteration and never empties its queue. A loop that drained the
    // check phase until it was empty would spin here forever and the timer would never fire.
    for (int i = 0; i < 3; ++i) {
        EXPECT_EQ(loop.RunOneIteration(), IterationResult::kRanTasks);
    }
    EXPECT_EQ(polls, 3);
    EXPECT_FALSE(timer_ran);

    loop.AdvanceBy(milliseconds(10));
    EXPECT_EQ(loop.RunOneIteration(), IterationResult::kRanTasks);
    EXPECT_TRUE(timer_ran);
    EXPECT_EQ(polls, 4);
}

TEST_F(EventLoopTest, CancelDelayedTaskPreventsTheTaskFromRunning) {
    TestEventLoop loop(isolate_, platform_);
    std::vector<std::string> order;

    loop.PostDelayedTask([&] { order.push_back("kept"); }, milliseconds(10));
    const EventLoop::TimerId cancelled =
        loop.PostDelayedTask([&] { order.push_back("cancelled"); }, milliseconds(10));
    loop.CancelDelayedTask(cancelled);

    loop.AdvanceBy(milliseconds(10));
    EXPECT_EQ(loop.RunOneIteration(), IterationResult::kRanTasks);
    EXPECT_EQ(Join(order), "kept");
}

TEST_F(EventLoopTest, CancellingAnUnknownOrExpiredTimerIsIgnored) {
    TestEventLoop loop(isolate_, platform_);
    bool ran = false;

    const EventLoop::TimerId id = loop.PostDelayedTask([&] { ran = true; }, milliseconds(0));
    EXPECT_EQ(loop.RunOneIteration(), IterationResult::kRanTasks);
    EXPECT_TRUE(ran);

    // clearTimeout() on a timer that has already fired, and on one that never existed, are both
    // no-ops rather than errors.
    loop.CancelDelayedTask(id);
    loop.CancelDelayedTask(id + 1000);
    EXPECT_EQ(loop.RunOneIteration(), IterationResult::kIdle);
}

TEST_F(EventLoopTest, ANegativeDelayIsTreatedAsZero) {
    TestEventLoop loop(isolate_, platform_);
    bool ran = false;

    loop.PostDelayedTask([&] { ran = true; }, milliseconds(-100));

    EXPECT_EQ(loop.RunOneIteration(), IterationResult::kRanTasks);
    EXPECT_TRUE(ran);
}

TEST_F(EventLoopTest, AnIterationWithNothingToDoReportsIdle) {
    TestEventLoop loop(isolate_, platform_);

    EXPECT_EQ(loop.RunOneIteration(), IterationResult::kIdle);
    EXPECT_FALSE(loop.NextDueTime().has_value());
}

TEST_F(EventLoopTest, NextDueTimeReportsTheEarliestTimer) {
    TestEventLoop loop(isolate_, platform_);

    loop.PostDelayedTask([] {}, milliseconds(50));
    const std::optional<EventLoop::TimePoint> later = loop.NextDueTime();
    ASSERT_TRUE(later.has_value());

    loop.PostDelayedTask([] {}, milliseconds(10));
    const std::optional<EventLoop::TimePoint> earlier = loop.NextDueTime();
    ASSERT_TRUE(earlier.has_value());

    EXPECT_LT(*earlier, *later);
}

TEST_F(EventLoopTest, MicrotasksRunBetweenTasksRatherThanAfterThem) {
    TestEventLoop loop(isolate_, platform_);
    std::vector<std::string> order;
    InstallRecorder(&order);

    loop.PostTask([&] {
        order.push_back("first task");
        RunScript("Promise.resolve().then(() => record('microtask'));");
    });
    loop.PostTask([&] { order.push_back("second task"); });

    EXPECT_EQ(loop.RunOneIteration(), IterationResult::kRanTasks);
    EXPECT_EQ(Join(order), "first task,microtask,second task");
}

TEST_F(EventLoopTest, AnUncaughtExceptionIsReportedAndStopsTheLoop) {
    TestEventLoop loop(isolate_, platform_);
    std::vector<std::string> order;

    loop.PostTask([&] {
        order.push_back("throws");
        RunScript("throw new Error('boom');");
    });
    loop.PostTask([&] { order.push_back("never runs"); });

    StderrCapture stderr_capture;
    EXPECT_EQ(loop.RunOneIteration(), IterationResult::kStopped);

    EXPECT_NE(stderr_capture.str().find("boom"), std::string::npos);
    EXPECT_EQ(Join(order), "throws");
    EXPECT_TRUE(loop.stopped());
    EXPECT_EQ(loop.exit_code(), 1);
}

TEST_F(EventLoopTest, StopFromInsideATaskAbandonsTheRemainingImmediates) {
    TestEventLoop loop(isolate_, platform_);
    std::vector<std::string> order;

    loop.PostTask([&] {
        order.push_back("first");
        loop.Stop(0);
    });
    loop.PostTask([&] { order.push_back("second"); });

    EXPECT_EQ(loop.RunOneIteration(), IterationResult::kStopped);
    EXPECT_EQ(Join(order), "first");
}

// Run() sleeps against the real clock, so the tests below use a plain EventLoop with delays short
// enough to be imperceptible. They assert only the order things happened in, never how long
// anything took, so a loaded machine cannot fail them.
constexpr auto kShortDelay = milliseconds(1);

TEST_F(EventLoopTest, RunReturnsWhenNoWorkRemains) {
    EventLoop loop(isolate_, platform_);
    std::vector<std::string> order;

    loop.PostTask([&] { order.push_back("immediate"); });
    loop.PostDelayedTask([&] { order.push_back("timer"); }, kShortDelay);

    loop.Run();

    // The immediate is ready straight away; reaching the timer means the loop waited for it.
    EXPECT_EQ(Join(order), "immediate,timer");
    EXPECT_FALSE(loop.stopped());
}

TEST_F(EventLoopTest, RunDrivesAChainOfTimers) {
    EventLoop loop(isolate_, platform_);
    std::vector<std::string> order;

    loop.PostDelayedTask(
        [&] {
            order.push_back("first");
            loop.PostDelayedTask([&] { order.push_back("second"); }, kShortDelay);
        },
        kShortDelay);

    loop.Run();

    EXPECT_EQ(Join(order), "first,second");
}

TEST_F(EventLoopTest, RunStopsAndKeepsTheExitCode) {
    EventLoop loop(isolate_, platform_);
    std::vector<std::string> order;
    int polls = 0;

    loop.PostTask([&] {
        order.push_back("ran");
        loop.Stop(3);
    });
    // Would keep the loop alive forever if Stop() were ignored.
    PostSelfRePostingTask(&loop, &polls);

    loop.Run();

    EXPECT_EQ(Join(order), "ran");
    EXPECT_EQ(polls, 0);
    EXPECT_TRUE(loop.stopped());
    EXPECT_EQ(loop.exit_code(), 3);
}

}  // namespace

}  // namespace dawn::node::standalone

#pragma clang diagnostic pop
