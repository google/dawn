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

#include "src/dawn/node/standalone/EventLoop.h"

#include <algorithm>
#include <cassert>
#include <deque>
#include <iostream>
#include <optional>
#include <thread>
#include <utility>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wundef"
#pragma clang diagnostic ignored "-Wcast-function-type-strict"
#pragma clang diagnostic ignored "-Wcast-function-type-mismatch"
#pragma clang diagnostic ignored "-Wsuggest-destructor-override"
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
#pragma clang diagnostic ignored "-Wunique-object-duplication"
#pragma clang diagnostic ignored "-Wundefined-reinterpret-cast"
#include <v8.h>

#include "libplatform/libplatform.h"
#pragma clang diagnostic pop

namespace dawn::node::standalone {

namespace {

constexpr auto kMaxSleep = std::chrono::milliseconds(10);

// Print an uncaught exception to stderr.
void ReportException(v8::Isolate* isolate, v8::TryCatch* try_catch) {
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Local<v8::Value> exception = try_catch->Exception();

    v8::MaybeLocal<v8::Value> maybe_stack = try_catch->StackTrace(context);
    if (!maybe_stack.IsEmpty()) {
        v8::Local<v8::Value> stack_val = maybe_stack.ToLocalChecked();
        if (stack_val->IsString()) {
            v8::String::Utf8Value stack_str(isolate, stack_val);
            if (stack_str.length() > 0 && *stack_str != nullptr) {
                std::cerr << *stack_str << std::endl;
                return;
            }
        }
    }

    v8::Local<v8::Message> message = try_catch->Message();
    if (!message.IsEmpty()) {
        v8::String::Utf8Value filename(isolate, message->GetScriptOrigin().ResourceName());
        int line_num = message->GetLineNumber(context).FromMaybe(0);
        v8::String::Utf8Value exception_str(isolate, exception);
        std::cerr << (*filename ? *filename : "<unknown>") << ":" << line_num << ": "
                  << (*exception_str ? *exception_str : "") << std::endl;
    } else {
        // V8 omits the message only for a terminated execution, whose value prints as "null".
        v8::String::Utf8Value exception_str(isolate, exception);
        std::cerr << "Uncaught exception: " << (*exception_str ? *exception_str : "") << std::endl;
    }
}

}  // namespace

EventLoop::EventLoop(v8::Isolate* isolate, v8::Platform* platform)
    : isolate_(isolate), platform_(platform) {}

EventLoop::~EventLoop() = default;

void EventLoop::AssertOnLoopThread() const {
    assert(std::this_thread::get_id() == thread_id_);
}

void EventLoop::PostTask(Task task) {
    AssertOnLoopThread();
    immediates_.push_back(std::move(task));
}

EventLoop::TimerId EventLoop::PostDelayedTask(Task task, Duration delay) {
    AssertOnLoopThread();
    const TimePoint due = Now() + std::max(delay, Duration::zero());
    const TimerId id = next_timer_id_++;
    timers_.emplace(TimerKey{due, id}, std::move(task));
    return id;
}

void EventLoop::CancelDelayedTask(TimerId id) {
    AssertOnLoopThread();
    const auto it =
        std::ranges::find_if(timers_, [id](const auto& entry) { return entry.first.id == id; });
    if (it != timers_.end()) {
        timers_.erase(it);
    }
}

EventLoop::IterationResult EventLoop::RunOneIteration() {
    AssertOnLoopThread();
    if (stopped_) {
        return IterationResult::kStopped;
    }

    bool ran_task = PumpEngineTasks();

    // Timers phase. The phase runs the timers that were already posted and already due when it
    // began, and nothing else, so that a timer whose callback arms another timer cannot hold the
    // loop here. A timer armed from inside the phase never sorts before the cutoff: its id is at
    // least the cutoff id, and its due time is at least the cutoff time because the clock only
    // advances. So the first entry that is not below the cutoff ends the phase.
    const TimerKey cutoff = {Now(), next_timer_id_};
    while (!stopped_) {
        const auto it = timers_.begin();
        if (it == timers_.end() || it->first >= cutoff) {
            break;
        }
        // Take the task out and drop its entry before running it, since running it may arm or
        // cancel timers and invalidate `it`.
        Task task = std::move(it->second);
        timers_.erase(it);
        RunTask(std::move(task));
        ran_task = true;
    }
    if (stopped_) {
        return IterationResult::kStopped;
    }

    // Check phase. Taking the whole queue up front is what makes an immediate posted from inside
    // this phase wait for the next iteration.
    //
    // That deferral is load bearing. Dawn's AsyncRunner polls for completed work by posting a
    // fresh immediate from inside its own immediate callback, so a loop that kept draining the
    // queue until it emptied would never leave the check phase and would starve every timer.
    std::deque<Task> immediates;
    immediates.swap(immediates_);
    while (!immediates.empty() && !stopped_) {
        Task immediate = std::move(immediates.front());
        immediates.pop_front();
        RunTask(std::move(immediate));
        ran_task = true;
    }
    if (stopped_) {
        return IterationResult::kStopped;
    }

    return ran_task ? IterationResult::kRanTasks : IterationResult::kIdle;
}

void EventLoop::Run() {
    AssertOnLoopThread();
    while (true) {
        const IterationResult result = RunOneIteration();
        if (result == IterationResult::kStopped) {
            return;
        }
        if (result == IterationResult::kRanTasks) {
            continue;
        }

        // Idle. No immediate can be waiting, because only a task posts one and no task ran, so a
        // timer is the only thing left that could become ready.
        const std::optional<TimePoint> due = NextDueTime();
        if (!due.has_value()) {
            // Nothing can ever become ready: the program has run to completion.
            return;
        }
        WaitUntil(*due);
    }
}

void EventLoop::Stop(int exit_code) {
    AssertOnLoopThread();
    stopped_ = true;
    exit_code_ = exit_code;
}

void EventLoop::RunTask(Task task) {
    {
        v8::HandleScope task_scope(isolate_);
        v8::TryCatch try_catch(isolate_);
        task();
        if (try_catch.HasCaught()) {
            ReportException(isolate_, &try_catch);
            Stop(1);
            return;
        }
    }
    // Outside the handle scope, since a microtask is a separate turn and opens its own.
    isolate_->PerformMicrotaskCheckpoint();
}

void EventLoop::WaitUntil(TimePoint due) {
    const TimePoint now = Now();
    if (due > now) {
        std::this_thread::sleep_for(std::min<Duration>(due - now, kMaxSleep));
    }
}

bool EventLoop::PumpEngineTasks() {
    return v8::platform::PumpMessageLoop(platform_, isolate_);
}

EventLoop::TimePoint EventLoop::Now() const {
    return Clock::now();
}

}  // namespace dawn::node::standalone
