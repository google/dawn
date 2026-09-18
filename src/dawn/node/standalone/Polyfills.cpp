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

#include "src/dawn/node/standalone/Polyfills.h"

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <system_error>
#include <vector>

namespace dawn::node::standalone {

namespace {

// State shared by the polyfills that outlives registration. Owned by the `process` object, which
// deletes it from its finalizer.
struct PolyfillContext {
    PolyfillOptions options;
    std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();
};

void DeletePolyfillContext(const Napi::Env&, PolyfillContext* ctx) {
    delete ctx;
}

// Joins the call's arguments with spaces, as the console methods display them.
Napi::Value FormatArgs(const Napi::CallbackInfo& info) {
    std::stringstream ss;
    for (size_t i = 0; i < info.Length(); ++i) {
        if (i > 0) {
            ss << " ";
        }
        ss << info[i].ToString().Utf8Value();
    }
    return Napi::String::New(info.Env(), ss.str());
}

// ---------------------------------------------------------------------------
// console
// ---------------------------------------------------------------------------

// Backs console.log(), console.info() and console.debug(). Node defines the latter two as
// aliases of the first, all writing to stdout with no prefix:
// https://nodejs.org/api/console.html#consoledebugdata-args
Napi::Value ConsoleLog(const Napi::CallbackInfo& info) {
    std::cout << FormatArgs(info).As<Napi::String>().Utf8Value() << std::endl;
    return info.Env().Undefined();
}

Napi::Value ConsoleWarn(const Napi::CallbackInfo& info) {
    std::clog << "[WARN] " << FormatArgs(info).As<Napi::String>().Utf8Value() << std::endl;
    return info.Env().Undefined();
}

Napi::Value ConsoleError(const Napi::CallbackInfo& info) {
    std::cerr << "[ERROR] " << FormatArgs(info).As<Napi::String>().Utf8Value() << std::endl;
    return info.Env().Undefined();
}

// https://developer.mozilla.org/en-US/docs/Web/API/console
void RegisterConsole(Napi::Env env) {
    Napi::Object console = Napi::Object::New(env);
    console.Set("log", Napi::Function::New(env, ConsoleLog, "log"));
    console.Set("info", Napi::Function::New(env, ConsoleLog, "info"));
    console.Set("debug", Napi::Function::New(env, ConsoleLog, "debug"));
    console.Set("warn", Napi::Function::New(env, ConsoleWarn, "warn"));
    console.Set("error", Napi::Function::New(env, ConsoleError, "error"));
    env.Global().Set("console", console);
}

// ---------------------------------------------------------------------------
// process
// ---------------------------------------------------------------------------

Napi::Value Cwd(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    std::error_code ec;
    std::filesystem::path cwd = std::filesystem::current_path(ec);
    if (ec) {
        return Napi::String::New(env, "");
    }
    return Napi::String::New(env, cwd.string());
}

Napi::Value Exit(const Napi::CallbackInfo& info) {
    auto* ctx = static_cast<PolyfillContext*>(info.Data());
    int32_t code = 0;
    if (info.Length() > 0 && info[0].IsNumber()) {
        code = info[0].As<Napi::Number>().Int32Value();
    }
    if (ctx != nullptr && ctx->options.on_exit) {
        ctx->options.on_exit(code);
    } else {
        std::exit(code);
    }
    return info.Env().Undefined();
}

Napi::Value HrtimeBigint(const Napi::CallbackInfo& info) {
    auto now = std::chrono::steady_clock::now().time_since_epoch();
    int64_t nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
    return Napi::BigInt::New(info.Env(), nanos);
}

// https://nodejs.org/api/stream.html#writablewritechunk-encoding-callback
//
// write() returns whether the caller may continue writing immediately, or should wait for a
// 'drain' event because the stream buffered the chunk. These writes go straight to the underlying
// stream and buffer nothing, so the answer is always true.
Napi::Value StdoutWrite(const Napi::CallbackInfo& info) {
    if (info.Length() > 0) {
        std::cout << info[0].ToString().Utf8Value() << std::flush;
    }
    return Napi::Boolean::New(info.Env(), true);
}

Napi::Value StderrWrite(const Napi::CallbackInfo& info) {
    if (info.Length() > 0) {
        std::clog << info[0].ToString().Utf8Value() << std::flush;
    }
    return Napi::Boolean::New(info.Env(), true);
}

// https://nodejs.org/api/process.html
void RegisterProcess(Napi::Env env, const PolyfillOptions& options, PolyfillContext* ctx) {
    Napi::Object process = Napi::Object::New(env);
    process.Set("cwd", Napi::Function::New(env, Cwd, "cwd"));
    process.Set("exit", Napi::Function::New(env, Exit, "exit", ctx));

    Napi::Object hrtime = Napi::Object::New(env);
    hrtime.Set("bigint", Napi::Function::New(env, HrtimeBigint, "bigint"));
    process.Set("hrtime", hrtime);

    Napi::Object env_obj = Napi::Object::New(env);
    if (const char* dawn_flags = std::getenv("DAWN_FLAGS")) {
        env_obj.Set("DAWN_FLAGS", Napi::String::New(env, dawn_flags));
    }
    process.Set("env", env_obj);

    Napi::Array argv_array = Napi::Array::New(env, options.argv.size());
    for (size_t i = 0; i < options.argv.size(); ++i) {
        argv_array.Set(i, Napi::String::New(env, options.argv[i]));
    }
    process.Set("argv", argv_array);

    Napi::Object stdout_obj = Napi::Object::New(env);
    stdout_obj.Set("write", Napi::Function::New(env, StdoutWrite));
    process.Set("stdout", stdout_obj);

    Napi::Object stderr_obj = Napi::Object::New(env);
    stderr_obj.Set("write", Napi::Function::New(env, StderrWrite));
    process.Set("stderr", stderr_obj);

    // `process` owns the context: this releases it once the object is collected.
    process.AddFinalizer(DeletePolyfillContext, ctx);

    env.Global().Set("process", process);
}

// ---------------------------------------------------------------------------
// performance
// ---------------------------------------------------------------------------

Napi::Value PerformanceNow(const Napi::CallbackInfo& info) {
    auto* ctx = static_cast<PolyfillContext*>(info.Data());
    auto now = std::chrono::steady_clock::now();
    double millis = 0.0;
    if (ctx != nullptr) {
        millis = std::chrono::duration<double, std::milli>(now - ctx->start_time).count();
    }
    return Napi::Number::New(info.Env(), millis);
}

// https://developer.mozilla.org/en-US/docs/Web/API/Performance/now
void RegisterPerformance(Napi::Env env, PolyfillContext* ctx) {
    Napi::Object performance = Napi::Object::New(env);
    performance.Set("now", Napi::Function::New(env, PerformanceNow, "now", ctx));
    env.Global().Set("performance", performance);
}

// ---------------------------------------------------------------------------
// bootstrap
// ---------------------------------------------------------------------------

const char* kBootstrapScript = R"bootstrap(
(function() {
})();
)bootstrap";

// Runs the JavaScript half of the polyfills, for the globals that are simpler to express in
// script than to assemble through the C++ API.
void RunBootstrapScript(Napi::Env env) {
    napi_value script_src;
    napi_create_string_utf8(env, kBootstrapScript, NAPI_AUTO_LENGTH, &script_src);
    napi_value result;
    napi_run_script(env, script_src, &result);
}

}  // namespace

void RegisterPolyfills(Napi::Env env, const PolyfillOptions& options) {
    auto* ctx = new PolyfillContext{options};

    RegisterConsole(env);
    RegisterProcess(env, options, ctx);
    RegisterPerformance(env, ctx);
    RunBootstrapScript(env);
}

}  // namespace dawn::node::standalone
