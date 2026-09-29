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

#include <filesystem>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include "src/dawn/node/Module.h"
#include "src/dawn/node/napi_v8/napi_v8.h"
#include "src/dawn/node/standalone/EventLoop.h"
#include "src/dawn/node/standalone/Polyfills.h"
#include "src/dawn/utils/CommandLineParser.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wundef"
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
#include "libplatform/libplatform.h"
#pragma clang diagnostic pop

namespace {

// Relative path from the CTS directory to its command-line entry point. Must start with `./` so
// the CommonJS `require()` polyfill resolves it as a relative file path rather than rejecting it
// as a bare package name.
constexpr const char* kCmdlineScript = "./out-node/common/runtime/cmdline.js";

class StandaloneRunner {
  public:
    int Run(int argc, const char* argv[]) {
        if (std::optional<int> exit_code = ParseCommandLine(argc, argv)) {
            return *exit_code;
        }

        InitializeV8();
        int exit_code = RunCts();
        DisposeV8();
        return exit_code;
    }

  private:
    // Parses command-line flags and switches the working directory to `--cts-dir`. Returns an exit
    // code if the process should terminate early (0 for `--help`, 1 on error), or `std::nullopt`
    // to continue running the CTS.
    std::optional<int> ParseCommandLine(int argc, const char* argv[]) {
        auto parse_result = parser_.Parse(argc, argv);
        if (!parse_result.success) {
            std::cerr << parse_result.errorMessage << "\n";
            return 1;
        }

        if (help_opt_.GetValue()) {
            std::cout << "Usage: " << argv[0] << " [options] --query <query>\n\nOptions:\n";
            parser_.PrintHelp(std::cout);
            return 0;
        }

        if (!query_opt_.IsSet()) {
            std::cerr << "Error: --query (-q) must be specified.\n\n";
            std::cerr << "Usage: " << argv[0] << " [options] --query <query>\n\nOptions:\n";
            parser_.PrintHelp(std::cerr);
            return 1;
        }

        // The CTS `cmdline.js` script requires the process working directory to be the root of the
        // CTS checkout: it checks that `src/common/runtime/cmdline.ts` exists relative to the
        // current working directory and loads test resources from `out-node/resources`.
        std::string cts_dir =
            cts_dir_opt_.IsSet() ? cts_dir_opt_.GetValue() : "third_party/webgpu-cts";
        std::error_code ec;
        std::filesystem::current_path(cts_dir, ec);
        if (ec) {
            std::cerr << "Error: failed to change directory to '" << cts_dir
                      << "': " << ec.message() << "\n";
            return 1;
        }

        if (!std::filesystem::exists("src/common/runtime/cmdline.ts")) {
            std::cerr << "Error: '" << cts_dir
                      << "' is not a WebGPU CTS checkout (missing 'src/common/runtime/cmdline.ts')."
                         " Use --cts-dir to specify the path to the WebGPU CTS repository.\n";
            return 1;
        }

        if (!std::filesystem::exists(kCmdlineScript) ||
            !std::filesystem::exists("out-node/resources")) {
            std::cerr << "Error: the WebGPU CTS in '" << cts_dir
                      << "' has not been built (missing 'out-node/common/runtime/cmdline.js' or "
                         "'out-node/resources'). Build it by running 'npm ci && npx grunt node' in "
                         "'"
                      << cts_dir << "'.\n";
            return 1;
        }

        return std::nullopt;
    }

    // Builds the `process.argv` array expected by the CTS `cmdline.js` script from the parsed
    // command-line options.
    std::vector<std::string> BuildJsArgs() const {
        std::vector<std::string> js_args = {
            "dawn_standalone_cts_runner",
            kCmdlineScript,
            "--gpu-provider",
            "dawn.node",
        };

        const dawn::utils::CommandLineParser::BoolOption* bool_forwarding_opts[] = {
            &compat_opt_,
            &verbose_opt_,
            &list_opt_,
            &debug_opt_,
            &quiet_opt_,
            &unroll_const_eval_loops_opt_,
            &enforce_default_limits_opt_,
            &block_all_features_opt_,
        };
        for (const auto* opt : bool_forwarding_opts) {
            if (opt->GetValue()) {
                js_args.push_back("--" + opt->GetName());
            }
        }

        // CommandLineParser::StringListOption splits on ',', so rejoin comma-separated values that
        // belong to the preceding <key>=<value> flag.
        std::vector<std::string> dawn_flags;
        for (const std::string& token : dawn_flags_opt_.GetValue()) {
            if (token.find('=') != std::string::npos || dawn_flags.empty()) {
                dawn_flags.push_back(token);
            } else {
                dawn_flags.back() += "," + token;
            }
        }
        for (const std::string& flag : dawn_flags) {
            js_args.push_back("--gpu-provider-flag");
            js_args.push_back(flag);
        }

        js_args.push_back(query_opt_.GetValue());
        return js_args;
    }

    void InitializeV8() {
        v8::V8::SetFlagsFromString("--expose_gc");
        platform_ = v8::platform::NewDefaultPlatform();
        v8::V8::InitializePlatform(platform_.get());
        v8::V8::Initialize();

        allocator_.reset(v8::ArrayBuffer::Allocator::NewDefaultAllocator());
        v8::Isolate::CreateParams create_params;
        create_params.array_buffer_allocator = allocator_.get();
        isolate_ = v8::Isolate::New(create_params);
    }

    void DisposeV8() {
        isolate_->Dispose();
        isolate_ = nullptr;
        allocator_.reset();
        v8::V8::Dispose();
        v8::V8::DisposePlatform();
        platform_.reset();
    }

    void InitializePolyfills(Napi::Env env, dawn::node::standalone::EventLoop& loop) {
        dawn::node::standalone::PolyfillOptions polyfill_options;
        polyfill_options.argv = BuildJsArgs();
        dawn::node::standalone::RegisterPolyfills(env, loop, polyfill_options);
    }

    // Initializes the built-in Dawn WebGPU module and exposes its WebGPU types on the JavaScript
    // global object.
    void InitializeWebGPU(Napi::Env env) {
        Napi::Object wgpu_exports = Napi::Object::New(env);
        Initialize(env, wgpu_exports);

        Napi::Object js_global = env.Global();
        js_global.Set("_webgpu_module", wgpu_exports);

        // `Initialize()` exports the WebGPU constructors and namespace objects (`GPUBufferUsage`,
        // `GPUShaderStage`, `GPUValidationError`, etc.) on the `"globals"` property of
        // `wgpu_exports`. Expose each of those properties on `js_global`.
        Napi::Value wgpu_types_val = wgpu_exports.Get("globals");
        if (wgpu_types_val.IsObject()) {
            Napi::Object wgpu_types = wgpu_types_val.As<Napi::Object>();
            Napi::Array names = wgpu_types.GetPropertyNames();
            for (uint32_t i = 0; i < names.Length(); ++i) {
                Napi::Value name = names.Get(i);
                js_global.Set(name, wgpu_types.Get(name));
            }
        }
    }

    // Sets up the Node-API environment, loads the CTS command-line entry point, and runs the event
    // loop to completion.
    int RunCts() {
        int exit_code = 0;
        isolate_->Enter();
        // `handle_scope` must be destroyed before `isolate_->Exit()`.
        {
            v8::HandleScope handle_scope(isolate_);
            v8::Local<v8::Context> context = v8::Context::New(isolate_);
            context->Enter();

            napi_env c_env = dawn::napi_v8::CreateEnv(isolate_, context);
            // `loop` must be destroyed before `DestroyEnv(c_env)` because queued tasks hold
            // `Napi::Reference`s that unregister themselves from `c_env` on destruction.
            {
                Napi::Env env(c_env);
                dawn::node::standalone::EventLoop loop(isolate_, platform_.get());

                // `InitializePolyfills` requires `loop` to bind timers and `process.exit`, and
                // must run before `InitializeWebGPU` which subclasses `Event` and `DOMException`.
                InitializePolyfills(env, loop);
                InitializeWebGPU(env);

                loop.PostTask([env] {
                    Napi::Function require_fn = env.Global().Get("require").As<Napi::Function>();
                    require_fn.Call({Napi::String::New(env, kCmdlineScript)});
                });
                loop.Run();
                exit_code = loop.exit_code();
            }
            dawn::napi_v8::DestroyEnv(c_env);

            context->Exit();
        }
        isolate_->Exit();
        return exit_code;
    }

    dawn::utils::CommandLineParser parser_;
    dawn::utils::CommandLineParser::BoolOption& help_opt_ = parser_.AddHelp();
    dawn::utils::CommandLineParser::StringOption& cts_dir_opt_ =
        parser_
            .AddString("cts-dir",
                       "Directory containing the WebGPU CTS (default: third_party/webgpu-cts)")
            .Parameter("dir");
    dawn::utils::CommandLineParser::StringOption& query_opt_ =
        parser_.AddString("query", "CTS test query to run (e.g. 'webgpu:*')")
            .ShortName('q')
            .Parameter("query");
    dawn::utils::CommandLineParser::BoolOption& verbose_opt_ =
        parser_.AddBool("verbose", "Print result/log of every test as it runs").ShortName('v');
    dawn::utils::CommandLineParser::BoolOption& quiet_opt_ =
        parser_.AddBool("quiet", "Suppress summary information in output");
    dawn::utils::CommandLineParser::BoolOption& list_opt_ =
        parser_.AddBool("list", "Print all testcase names that match the given query and exit");
    dawn::utils::CommandLineParser::BoolOption& debug_opt_ =
        parser_.AddBool("debug", "Include debug messages in logging");
    dawn::utils::CommandLineParser::BoolOption& compat_opt_ =
        parser_.AddBool("compat", "Run tests in compatibility mode");
    dawn::utils::CommandLineParser::BoolOption& unroll_const_eval_loops_opt_ =
        parser_.AddBool("unroll-const-eval-loops",
                        "Unroll loops in constant-evaluation shader execution tests");
    dawn::utils::CommandLineParser::BoolOption& enforce_default_limits_opt_ =
        parser_.AddBool("enforce-default-limits", "Enforce the default limits");
    dawn::utils::CommandLineParser::BoolOption& block_all_features_opt_ =
        parser_.AddBool("block-all-features",
                        "Block all features (except 'core-features-and-limits')");
    dawn::utils::CommandLineParser::StringListOption& dawn_flags_opt_ =
        parser_.AddStringList("dawn-flag", "Flag to set on dawn.node as <flag>=<value>")
            .Parameter("flag=value");

    std::unique_ptr<v8::Platform> platform_;
    std::unique_ptr<v8::ArrayBuffer::Allocator> allocator_;
    v8::Isolate* isolate_ = nullptr;
};

}  // namespace

int main(int argc, const char* argv[]) {
    StandaloneRunner runner;
    return runner.Run(argc, argv);
}
