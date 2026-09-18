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
#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>

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

#include "src/dawn/common/SystemUtils.h"
#include "src/dawn/node/napi_v8/napi_v8.h"
#include "src/dawn/node/standalone/Polyfills.h"

namespace {

// How long the clock tests sleep, and the bounds a reading taken across that sleep must fall
// within. The lower bound sits under the sleep to leave room for clock-source rounding; the upper
// is loose enough that only a wrong unit, rather than a loaded machine, can reach it.
constexpr std::chrono::milliseconds kSleepDuration{50};
constexpr double kMinElapsedMs = 40.0;
constexpr double kMaxElapsedMs = 5000.0;

// Redirects `stream` into a buffer for the lifetime of this object, so that tests can assert on
// the exact bytes a polyfill writes rather than merely that it did not crash.
class StreamCapture {
  public:
    explicit StreamCapture(std::ostream& stream) : stream_(stream) {
        original_ = stream_.rdbuf(buffer_.rdbuf());
    }

    ~StreamCapture() { stream_.rdbuf(original_); }

    StreamCapture(const StreamCapture&) = delete;
    StreamCapture& operator=(const StreamCapture&) = delete;

    std::string Str() const { return buffer_.str(); }

  private:
    std::ostringstream buffer_;
    std::ostream& stream_;
    std::streambuf* original_;
};

class PolyfillsTest : public ::testing::Test {
  protected:
    void SetUp() override {
        allocator_.reset(v8::ArrayBuffer::Allocator::NewDefaultAllocator());
        create_params_.array_buffer_allocator = allocator_.get();
        isolate_ = v8::Isolate::New(create_params_);
        isolate_->SetMicrotasksPolicy(v8::MicrotasksPolicy::kExplicit);
        isolate_->Enter();

        handle_scope_.emplace(isolate_);

        v8::Local<v8::Context> context = v8::Context::New(isolate_);
        context->Enter();
        env_ = dawn::napi_v8::CreateEnv(isolate_, context);
    }

    void TearDown() override {
        env_->GetContext()->Exit();
        dawn::napi_v8::DestroyEnv(env_);
        handle_scope_.reset();
        isolate_->Exit();
        isolate_->Dispose();
        allocator_.reset();
    }

    napi_value RunScript(const std::string& code) {
        napi_value script_src;
        napi_value result;
        napi_status status =
            napi_create_string_utf8(env_, code.c_str(), code.length(), &script_src);
        EXPECT_EQ(status, napi_ok);
        status = napi_run_script(env_, script_src, &result);
        EXPECT_EQ(status, napi_ok);
        return result;
    }

    void RunMicrotasks() { isolate_->PerformMicrotaskCheckpoint(); }

    std::string ToString(napi_value value) {
        size_t length = 0;
        EXPECT_EQ(napi_get_value_string_utf8(env_, value, nullptr, 0, &length), napi_ok);
        std::vector<char> buffer(length + 1);
        EXPECT_EQ(napi_get_value_string_utf8(env_, value, buffer.data(), buffer.size(), &length),
                  napi_ok);
        return std::string(buffer.data(), length);
    }

    bool ToBool(napi_value value) {
        bool result = false;
        EXPECT_EQ(napi_get_value_bool(env_, value, &result), napi_ok);
        return result;
    }

    uint32_t ToUint32(napi_value value) {
        uint32_t result = 0;
        EXPECT_EQ(napi_get_value_uint32(env_, value, &result), napi_ok);
        return result;
    }

    double ToDouble(napi_value value) {
        double result = 0.0;
        EXPECT_EQ(napi_get_value_double(env_, value, &result), napi_ok);
        return result;
    }

    napi_env env_ = nullptr;

  private:
    std::unique_ptr<v8::ArrayBuffer::Allocator> allocator_;
    v8::Isolate::CreateParams create_params_;
    v8::Isolate* isolate_ = nullptr;
    std::optional<v8::HandleScope> handle_scope_;
};

TEST_F(PolyfillsTest, ConsoleGlobals) {
    dawn::node::standalone::RegisterPolyfills(env_);

    EXPECT_TRUE(
        ToBool(RunScript("typeof console.log === 'function' && "
                         "typeof console.warn === 'function' && "
                         "typeof console.error === 'function' && "
                         "typeof console.info === 'function' && "
                         "typeof console.debug === 'function'")));
}

TEST_F(PolyfillsTest, ConsoleLogInfoAndDebugWriteLinesToStdout) {
    dawn::node::standalone::RegisterPolyfills(env_);

    StreamCapture out(std::cout);
    StreamCapture log(std::clog);
    StreamCapture err(std::cerr);
    RunScript("console.log('one'); console.info('two'); console.debug('three');");

    // All three are unprefixed stdout writes, one line each.
    EXPECT_EQ(out.Str(), "one\ntwo\nthree\n");
    EXPECT_EQ(log.Str(), "");
    EXPECT_EQ(err.Str(), "");
}

TEST_F(PolyfillsTest, ConsoleWarnAndErrorAreLabelled) {
    dawn::node::standalone::RegisterPolyfills(env_);

    StreamCapture out(std::cout);
    StreamCapture log(std::clog);
    StreamCapture err(std::cerr);
    RunScript("console.warn('careful'); console.error('broken');");

    EXPECT_EQ(out.Str(), "");
    EXPECT_EQ(log.Str(), "[WARN] careful\n");
    EXPECT_EQ(err.Str(), "[ERROR] broken\n");
}

TEST_F(PolyfillsTest, ConsoleJoinsArgumentsWithSpaces) {
    dawn::node::standalone::RegisterPolyfills(env_);

    StreamCapture out(std::cout);
    RunScript("console.log('a', 1, true, null, undefined, {}, [1, 2]);");
    RunScript("console.log();");

    EXPECT_EQ(out.Str(), "a 1 true null undefined [object Object] 1,2\n\n");
}

TEST_F(PolyfillsTest, ProcessArgvAndCwd) {
    dawn::node::standalone::PolyfillOptions options;
    options.argv = {"runner", "arg1", "arg2"};
    dawn::node::standalone::RegisterPolyfills(env_, options);

    EXPECT_EQ(ToUint32(RunScript("process.argv.length")), 3u);
    EXPECT_EQ(ToString(RunScript("process.argv.join(',')")), "runner,arg1,arg2");

    // cwd() must report the real working directory, not merely something of type string.
    EXPECT_EQ(ToString(RunScript("process.cwd()")), std::filesystem::current_path().string());
}

TEST_F(PolyfillsTest, ProcessArgvIsEmptyByDefault) {
    dawn::node::standalone::RegisterPolyfills(env_);

    EXPECT_TRUE(ToBool(RunScript("Array.isArray(process.argv)")));
    EXPECT_EQ(ToUint32(RunScript("process.argv.length")), 0u);
}

TEST_F(PolyfillsTest, ProcessEnvExposesDawnFlags) {
    dawn::ScopedEnvironmentVar dawn_flags("DAWN_FLAGS", "--a-flag");
    dawn::node::standalone::RegisterPolyfills(env_);

    EXPECT_EQ(ToString(RunScript("process.env.DAWN_FLAGS")), "--a-flag");
}

TEST_F(PolyfillsTest, ProcessEnvIsEmptyWithoutDawnFlags) {
    dawn::ScopedEnvironmentVar dawn_flags("DAWN_FLAGS", nullptr);
    dawn::node::standalone::RegisterPolyfills(env_);

    // `process.env` is not the real environment: only the variables the runner reads are copied
    // into it, so with DAWN_FLAGS unset it is empty.
    EXPECT_EQ(ToUint32(RunScript("Object.keys(process.env).length")), 0u);
}

TEST_F(PolyfillsTest, ProcessExitCallback) {
    int32_t exit_code = -1;
    dawn::node::standalone::PolyfillOptions options;
    options.on_exit = [&](int32_t code) { exit_code = code; };
    dawn::node::standalone::RegisterPolyfills(env_, options);

    RunScript("process.exit(42)");
    EXPECT_EQ(exit_code, 42);
}

TEST_F(PolyfillsTest, ProcessExitDefaultsToZero) {
    int32_t exit_code = -1;
    dawn::node::standalone::PolyfillOptions options;
    options.on_exit = [&](int32_t code) { exit_code = code; };
    dawn::node::standalone::RegisterPolyfills(env_, options);

    RunScript("process.exit()");
    EXPECT_EQ(exit_code, 0);

    exit_code = -1;
    RunScript("process.exit('not a number')");
    EXPECT_EQ(exit_code, 0);
}

TEST_F(PolyfillsTest, ProcessStreamWritesAreVerbatim) {
    dawn::node::standalone::RegisterPolyfills(env_);

    StreamCapture out(std::cout);
    StreamCapture log(std::clog);
    RunScript("process.stdout.write('no'); process.stdout.write(' newline');");
    RunScript("process.stderr.write('unlabelled');");

    // Unlike console.log(), write() adds neither a newline nor a prefix.
    EXPECT_EQ(out.Str(), "no newline");
    EXPECT_EQ(log.Str(), "unlabelled");
}

TEST_F(PolyfillsTest, ProcessStreamWriteReturnsTrue) {
    dawn::node::standalone::RegisterPolyfills(env_);

    StreamCapture out(std::cout);
    StreamCapture log(std::clog);

    // write() reports whether the caller may write again immediately, rather than waiting for a
    // 'drain' event.
    EXPECT_TRUE(ToBool(RunScript("process.stdout.write('x')")));
    EXPECT_TRUE(ToBool(RunScript("process.stderr.write('x')")));
}

TEST_F(PolyfillsTest, PerformanceNow) {
    dawn::node::standalone::RegisterPolyfills(env_);

    EXPECT_EQ(ToString(RunScript("typeof performance.now()")), "number");

    double now = ToDouble(RunScript("performance.now()"));
    EXPECT_GE(now, 0.0);
    // now() counts from registration, so a test that has only just started cannot be minutes in.
    // A clock returning, say, milliseconds since the epoch would fail here.
    EXPECT_LT(now, 60000.0);
}

// These only establish that the clock never runs backwards. Sampling in a loop cannot show that
// it runs forwards at all, because performance.now() reports whole and fractional milliseconds
// and a thousand calls can finish inside one of them; PerformanceNowAdvances covers that.
TEST_F(PolyfillsTest, PerformanceNowIsMonotonic) {
    dawn::node::standalone::RegisterPolyfills(env_);

    EXPECT_TRUE(ToBool(RunScript(R"((function() {
        let last = performance.now();
        for (let i = 0; i < 1000; ++i) {
            const next = performance.now();
            if (next < last) {
                return false;
            }
            last = next;
        }
        return true;
    })())")));
}

TEST_F(PolyfillsTest, PerformanceNowAdvances) {
    dawn::node::standalone::RegisterPolyfills(env_);

    double before = ToDouble(RunScript("performance.now()"));
    std::this_thread::sleep_for(kSleepDuration);
    double elapsed_ms = ToDouble(RunScript("performance.now()")) - before;

    // sleep_for() blocks for at least the duration asked of it, so the clock really has moved.
    // Bounding the reading from above as well as below also pins the unit: a clock reporting
    // seconds would read 0.05 here, and one reporting microseconds 50000.
    EXPECT_GE(elapsed_ms, kMinElapsedMs);
    EXPECT_LT(elapsed_ms, kMaxElapsedMs);
}

TEST_F(PolyfillsTest, HrtimeBigintIsMonotonic) {
    dawn::node::standalone::RegisterPolyfills(env_);

    EXPECT_EQ(ToString(RunScript("typeof process.hrtime.bigint()")), "bigint");

    EXPECT_TRUE(ToBool(RunScript(R"((function() {
        let last = process.hrtime.bigint();
        for (let i = 0; i < 1000; ++i) {
            const next = process.hrtime.bigint();
            if (next < last) {
                return false;
            }
            last = next;
        }
        return true;
    })())")));
}

TEST_F(PolyfillsTest, HrtimeBigintAdvances) {
    dawn::node::standalone::RegisterPolyfills(env_);

    RunScript("globalThis.hrtimeStart = process.hrtime.bigint();");
    std::this_thread::sleep_for(kSleepDuration);

    // The difference is taken in BigInt and only then converted, so no precision is lost before
    // the division. As above, the upper bound is what pins the unit to nanoseconds.
    double elapsed_ms =
        ToDouble(RunScript("Number(process.hrtime.bigint() - globalThis.hrtimeStart) / 1e6"));
    EXPECT_GE(elapsed_ms, kMinElapsedMs);
    EXPECT_LT(elapsed_ms, kMaxElapsedMs);
}

}  // namespace

#pragma clang diagnostic pop
