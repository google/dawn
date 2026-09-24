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

#ifndef SRC_DAWN_NODE_STANDALONE_EVENTLOOP_H_
#define SRC_DAWN_NODE_STANDALONE_EVENTLOOP_H_

#include <chrono>
#include <cstdint>
#include <deque>
#include <optional>
#include <thread>
#include <utility>

#include "absl/container/btree_map.h"
#include "absl/functional/any_invocable.h"

namespace v8 {
class Isolate;
class Platform;
}  // namespace v8

namespace dawn::node::standalone {

// A Node.js-shaped event loop. Each iteration runs the timers that have come due, then the
// immediates that were already queued when the phase began.
//
// Every task runs inside a handle scope, with the microtask queue drained afterwards, so a task is
// free to call into JavaScript. An uncaught exception is printed and stops the loop with exit code
// 1, which is why the loop rather than its caller owns the exit code.
//
// Node's phase order is not an implementation detail we are free to change. `setTimeout` and
// `setImmediate` callbacks observe it, and Dawn's AsyncRunner relies on the check phase deferring
// work to the next iteration; see the comment on RunOneIteration(). The order is documented at
// https://nodejs.org/en/learn/asynchronous-work/event-loop-timers-and-nexttick
//
// Every method must be called on the thread that constructed the loop, which is the thread the
// isolate runs on. Tasks only ever arrive by way of a JavaScript call - the timer polyfills, and
// AsyncRunner, which posts by calling the global setImmediate() rather than by reaching in here -
// and a JavaScript call is already confined to that thread. Debug builds check this.
class EventLoop {
  public:
    using Task = absl::AnyInvocable<void()>;
    using Clock = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;
    using Duration = Clock::duration;
    using TimerId = uint64_t;

    // What a single iteration found to do.
    enum class IterationResult {
        // At least one task ran. More may already be ready.
        kRanTasks,
        // Nothing was ready to run. The caller may wait until NextDueTime().
        kIdle,
        // Stop() has been called.
        kStopped,
    };

    // `isolate` must be entered, with a context entered, whenever a task runs. `platform` is the
    // one the isolate was created with; the loop pumps its foreground work each iteration. Neither
    // is owned, and both must outlive the loop.
    EventLoop(v8::Isolate* isolate, v8::Platform* platform);
    virtual ~EventLoop();

    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(const EventLoop&) = delete;

    // Queues `task` for the next check phase. Backs setImmediate().
    void PostTask(Task task);

    // Queues `task` to run once `delay` has elapsed, and returns an identifier that
    // CancelDelayedTask() accepts. Backs setTimeout() and setInterval().
    TimerId PostDelayedTask(Task task, Duration delay);

    // Drops a delayed task that has not run yet. Unknown and already-run identifiers are ignored,
    // because clearTimeout() on an expired timer is not an error.
    void CancelDelayedTask(TimerId id);

    // Runs one timers phase followed by one check phase.
    IterationResult RunOneIteration();

    // Runs iterations until Stop() is called or there is no work left to wait for.
    void Run();

    // Asks the loop to finish. Takes effect between tasks, so the caller runs to completion.
    // Backs process.exit().
    void Stop(int exit_code);

    bool stopped() const { return stopped_; }
    int exit_code() const { return exit_code_; }

    // When the earliest pending timer comes due, or nullopt when no timer is pending.
    std::optional<TimePoint> NextDueTime() const {
        if (timers_.empty()) {
            return std::nullopt;
        }
        return timers_.begin()->first.due;
    }

  protected:
    // The loop's only reading of the clock. Virtual so that tests can control the passage of time
    // without depending on real time; every other seam this class used to have is gone.
    virtual TimePoint Now() const;

  private:
    // Runs a single task and then drains the microtask queue, mirroring what a browser or Node.js
    // does between task callbacks. An uncaught exception is reported and stops the loop.
    void RunTask(Task task);

    // Blocks until `due`, or until it is worth looking for work again. Capped so that
    // PumpEngineTasks() gets a turn while a distant timer is pending.
    void WaitUntil(TimePoint due);

    // Runs any foreground work V8 has queued for this thread, reporting whether it ran any.
    bool PumpEngineTasks();

    // Checks, in debug builds, that the caller is on the thread that constructed the loop.
    void AssertOnLoopThread() const;

    // Orders timers by the time they come due, and timers that share a deadline by the order they
    // were posted, since identifiers are handed out in that order.
    struct TimerKey {
        TimePoint due;
        TimerId id;
        auto operator<=>(const TimerKey&) const = default;
    };

    v8::Isolate* const isolate_;
    v8::Platform* const platform_;
    // The thread the loop belongs to. Only read by AssertOnLoopThread().
    [[maybe_unused]] const std::thread::id thread_id_ = std::this_thread::get_id();
    // Tasks waiting for the next check phase, in the order they were posted.
    std::deque<Task> immediates_;
    absl::btree_map<TimerKey, Task> timers_;
    TimerId next_timer_id_ = 1;
    bool stopped_ = false;
    int exit_code_ = 0;
};

}  // namespace dawn::node::standalone

#endif  // SRC_DAWN_NODE_STANDALONE_EVENTLOOP_H_
