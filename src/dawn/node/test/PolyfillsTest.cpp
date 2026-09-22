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

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>
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

// A directory of its own for a test that needs real files, removed when the test ends. The name
// carries the test that asked for it, and a counter keeps two of them apart.
class TempDir {
  public:
    TempDir() {
        static std::atomic<uint32_t> counter{0};
        const ::testing::TestInfo* info = ::testing::UnitTest::GetInstance()->current_test_info();
        std::filesystem::path base = std::filesystem::temp_directory_path();
        for (uint32_t attempt = 0; attempt < kMaxAttempts; ++attempt) {
            std::filesystem::path candidate =
                base /
                ("dawn_polyfills_" + std::string(info->name()) + "_" + std::to_string(counter++));
            std::error_code ec;
            if (std::filesystem::create_directory(candidate, ec)) {
                path_ = candidate;
                return;
            }
        }
        ADD_FAILURE() << "no temporary directory could be created under " << base;
    }

    ~TempDir() {
        std::error_code ec;
        std::filesystem::remove_all(path_, ec);
    }

    TempDir(const TempDir&) = delete;
    TempDir& operator=(const TempDir&) = delete;

    // Paths handed to JavaScript use forward slashes throughout: Windows accepts them, and unlike
    // a backslash they do not start an escape sequence when pasted into a string literal.
    std::string JsPath() const { return path_.generic_string(); }

    // Creates a file in the directory and returns its path, for JavaScript.
    std::string WriteFile(const std::string& name, const std::string& contents) const {
        std::filesystem::path file = path_ / name;
        std::ofstream out(file, std::ios::binary);
        out << contents;
        out.close();
        EXPECT_TRUE(out.good()) << "could not write " << file;
        return file.generic_string();
    }

  private:
    static constexpr uint32_t kMaxAttempts = 32;

    std::filesystem::path path_;
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

    // Whether evaluating `expression` throws an Error. RunScript() expects the script itself to
    // succeed, so the throw has to be caught in JavaScript rather than escaping to the fixture.
    bool Throws(const std::string& expression) {
        return ToBool(RunScript("(function() { try { " + expression +
                                "; return false; } catch (e) { return e instanceof Error; } })()"));
    }

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

TEST_F(PolyfillsTest, FsExistsSync) {
    dawn::node::standalone::RegisterPolyfills(env_);

    TempDir dir;
    std::string file = dir.WriteFile("present.txt", "x");

    EXPECT_TRUE(ToBool(RunScript("_fs_polyfill.existsSync('" + file + "')")));
    EXPECT_TRUE(ToBool(RunScript("_fs_polyfill.existsSync('" + dir.JsPath() + "')")));
    EXPECT_FALSE(ToBool(RunScript("_fs_polyfill.existsSync('" + dir.JsPath() + "/absent.txt')")));

    // Anything that is not a path answers false rather than throwing.
    EXPECT_FALSE(ToBool(RunScript("_fs_polyfill.existsSync(42)")));
    EXPECT_FALSE(ToBool(RunScript("_fs_polyfill.existsSync()")));
}

TEST_F(PolyfillsTest, FsReadFileSyncDecodesWithAnEncoding) {
    dawn::node::standalone::RegisterPolyfills(env_);

    TempDir dir;
    std::string file = dir.WriteFile("hello.txt", "Hello WebGPU\n");

    // The encoding may be given on its own or inside an options object.
    EXPECT_EQ(ToString(RunScript("_fs_polyfill.readFileSync('" + file + "', 'utf8')")),
              "Hello WebGPU\n");
    EXPECT_EQ(ToString(RunScript("_fs_polyfill.readFileSync('" + file + "', {encoding: 'utf8'})")),
              "Hello WebGPU\n");
    // All buffer encodings are case-insensitive.
    EXPECT_EQ(ToString(RunScript("_fs_polyfill.readFileSync('" + file + "', 'UTF-8')")),
              "Hello WebGPU\n");
    EXPECT_EQ(ToString(RunScript("_fs_polyfill.readFileSync('" + file + "', {encoding: 'Utf8'})")),
              "Hello WebGPU\n");
}

TEST_F(PolyfillsTest, FsReadFileSyncReturnsBytesWithoutAnEncoding) {
    dawn::node::standalone::RegisterPolyfills(env_);

    TempDir dir;
    // Bytes that are neither printable nor valid UTF-8, so a decoded read could not reproduce
    // them.
    std::string file = dir.WriteFile("bytes.bin", std::string("\x01\x02\x00\xff", 4));

    EXPECT_TRUE(
        ToBool(RunScript("_fs_polyfill.readFileSync('" + file + "') instanceof Uint8Array")));
    EXPECT_EQ(
        ToString(RunScript("Array.from(_fs_polyfill.readFileSync('" + file + "')).join(',')")),
        "1,2,0,255");
}

TEST_F(PolyfillsTest, FsReadFileSyncThrowsForAMissingFile) {
    dawn::node::standalone::RegisterPolyfills(env_);

    TempDir dir;

    EXPECT_TRUE(Throws("_fs_polyfill.readFileSync('" + dir.JsPath() + "/absent.txt')"));
}

TEST_F(PolyfillsTest, FsReadFileSyncPreservesNewlinesVerbatim) {
    dawn::node::standalone::RegisterPolyfills(env_);

    TempDir dir;
    std::string file = dir.WriteFile("crlf.txt", "a\r\nb\n");

    // The file is read as bytes whether or not it is decoded, so a CR survives. Opening in text
    // mode on Windows would drop it and leave us disagreeing with Node.
    EXPECT_EQ(ToString(RunScript("_fs_polyfill.readFileSync('" + file + "', 'utf8')")), "a\r\nb\n");
    EXPECT_EQ(
        ToString(RunScript("Array.from(_fs_polyfill.readFileSync('" + file + "')).join(',')")),
        "97,13,10,98,10");
}

TEST_F(PolyfillsTest, FsReadFileSyncHandlesAnEmptyFile) {
    dawn::node::standalone::RegisterPolyfills(env_);

    TempDir dir;
    std::string file = dir.WriteFile("empty.txt", "");

    EXPECT_EQ(ToString(RunScript("_fs_polyfill.readFileSync('" + file + "', 'utf8')")), "");
    EXPECT_EQ(ToUint32(RunScript("_fs_polyfill.readFileSync('" + file + "').length")), 0u);
}

// An option the polyfill cannot honour has to be reported, not ignored: quietly returning
// something that does not match what was asked for is worse than failing.
TEST_F(PolyfillsTest, FsRejectsUnsupportedOptions) {
    dawn::node::standalone::RegisterPolyfills(env_);

    TempDir dir;
    std::string file = dir.WriteFile("hello.txt", "Hello WebGPU");

    EXPECT_TRUE(Throws("_fs_polyfill.readFileSync('" + file + "', {flag: 'r'})"));
    EXPECT_TRUE(Throws("_fs_polyfill.readFileSync('" + file + "', 'latin1')"));
    EXPECT_TRUE(Throws("_fs_polyfill.readFileSync('" + file + "', 42)"));
    EXPECT_TRUE(Throws("_fs_polyfill.readdirSync('" + dir.JsPath() + "', {withFileTypes: true})"));
    EXPECT_TRUE(Throws("_fs_polyfill.statSync('" + file + "', {bigint: true})"));

    // An absent, undefined or null options argument is not an unsupported one.
    EXPECT_FALSE(Throws("_fs_polyfill.readFileSync('" + file + "')"));
    EXPECT_FALSE(Throws("_fs_polyfill.readFileSync('" + file + "', undefined)"));
    EXPECT_FALSE(Throws("_fs_polyfill.readFileSync('" + file + "', {encoding: null})"));
    EXPECT_FALSE(Throws("_fs_polyfill.readdirSync('" + dir.JsPath() + "', 'utf8')"));
}

// Node's readdirSync() answers with Buffers under the 'buffer' encoding, and its readFileSync()
// rejects that encoding outright. We reject it in both: there is no Buffer in this runtime, and
// handing back Uint8Arrays instead would break the first caller to call toString() on one.
TEST_F(PolyfillsTest, FsRejectsTheBufferEncoding) {
    dawn::node::standalone::RegisterPolyfills(env_);

    TempDir dir;
    std::string file = dir.WriteFile("hello.txt", "Hello WebGPU");

    EXPECT_TRUE(Throws("_fs_polyfill.readdirSync('" + dir.JsPath() + "', 'buffer')"));
    EXPECT_TRUE(Throws("_fs_polyfill.readdirSync('" + dir.JsPath() + "', {encoding: 'buffer'})"));
    EXPECT_TRUE(Throws("_fs_polyfill.readFileSync('" + file + "', 'buffer')"));

    // Bytes are still reachable from readFileSync, just by omitting the encoding rather than by
    // naming one, which is what Node does too.
    EXPECT_TRUE(
        ToBool(RunScript("_fs_polyfill.readFileSync('" + file + "') instanceof Uint8Array")));
}

TEST_F(PolyfillsTest, FsRejectsUncPaths) {
    dawn::node::standalone::RegisterPolyfills(env_);

    // Node accepts UNC paths; this runtime deliberately refuses them, because Node and
    // std::filesystem disagree about where a UNC root ends. String.raw keeps the backslashes away
    // from JavaScript's escape rules so the path reaches the polyfill exactly as written.
    EXPECT_TRUE(Throws(R"(_fs_polyfill.readFileSync(String.raw`\\server\share\a`))"));
    EXPECT_TRUE(Throws(R"(_fs_polyfill.readdirSync(String.raw`\\server\share`))"));
    EXPECT_TRUE(Throws(R"(_fs_polyfill.statSync(String.raw`\\server\share\a`))"));

    // existsSync() answers false rather than throwing for arguments it cannot use, but a UNC path
    // is not a question about whether something exists, so it is refused rather than reported
    // absent.
    EXPECT_TRUE(Throws(R"(_fs_polyfill.existsSync(String.raw`\\server\share\a`))"));

    // The device forms share the prefix and go the same way.
    EXPECT_TRUE(Throws(R"(_fs_polyfill.existsSync(String.raw`\\?\C:\a`))"));
    EXPECT_TRUE(Throws(R"(_fs_polyfill.existsSync(String.raw`\\.\PhysicalDrive0`))"));
}

TEST_F(PolyfillsTest, FsAcceptsPathsThatMerelyResembleUnc) {
    dawn::node::standalone::RegisterPolyfills(env_);

    TempDir dir;
    std::string file = dir.WriteFile("hello.txt", "Hello WebGPU");

    // It takes two leading separators to make a UNC path. One is not enough, and a backslash
    // anywhere else is just a character. None of these exist, but none of them are refused
    // either - the distinction being that a refusal throws.
    EXPECT_FALSE(ToBool(RunScript(R"(_fs_polyfill.existsSync(String.raw`\server\share`))")));
    EXPECT_FALSE(ToBool(RunScript(R"(_fs_polyfill.existsSync('a\\b'))")));

    // Where the two platforms part company. On Windows "//server/share" is UNC and refused; on
    // POSIX a leading "//" is a legal path that the path module already handles correctly, so it
    // is left alone.
#if defined(_WIN32)
    EXPECT_TRUE(Throws("_fs_polyfill.existsSync('//no/such/path')"));
#else
    EXPECT_FALSE(ToBool(RunScript("_fs_polyfill.existsSync('//no/such/path')")));
#endif

    // And an ordinary path is untouched by any of this.
    EXPECT_EQ(ToString(RunScript("_fs_polyfill.readFileSync('" + file + "', 'utf8')")),
              "Hello WebGPU");
}

TEST_F(PolyfillsTest, FsReaddirSync) {
    dawn::node::standalone::RegisterPolyfills(env_);

    TempDir dir;
    dir.WriteFile("a.txt", "a");
    dir.WriteFile("b.txt", "b");

    // The entries are names, not paths, and the order is the filesystem's.
    EXPECT_EQ(
        ToString(RunScript("_fs_polyfill.readdirSync('" + dir.JsPath() + "').sort().join(',')")),
        "a.txt,b.txt");
}

TEST_F(PolyfillsTest, FsStatSync) {
    dawn::node::standalone::RegisterPolyfills(env_);

    TempDir dir;
    std::string file = dir.WriteFile("file.txt", "x");

    EXPECT_TRUE(ToBool(RunScript("const f = _fs_polyfill.statSync('" + file +
                                 "'); f.isFile() && !f.isDirectory()")));
    EXPECT_TRUE(ToBool(RunScript("const d = _fs_polyfill.statSync('" + dir.JsPath() +
                                 "'); d.isDirectory() && !d.isFile()")));
}

TEST_F(PolyfillsTest, FsReadFileCallbackRunsAsynchronously) {
    dawn::node::standalone::RegisterPolyfills(env_);

    TempDir dir;
    std::string file = dir.WriteFile("hello.txt", "Hello WebGPU");

    RunScript(
        "globalThis.error = 'unset'; globalThis.data = 'unset';"
        "_fs_polyfill.readFile('" +
        file + "', 'utf8', (err, data) => { globalThis.error = err; globalThis.data = data; });");

    // Nothing has happened yet: that the callback waits for the microtask checkpoint is what
    // makes this the asynchronous form.
    EXPECT_EQ(ToString(RunScript("globalThis.data")), "unset");

    RunMicrotasks();
    EXPECT_TRUE(ToBool(RunScript("globalThis.error === null")));
    EXPECT_EQ(ToString(RunScript("globalThis.data")), "Hello WebGPU");
}

TEST_F(PolyfillsTest, FsReadFileCallbackReportsFailure) {
    dawn::node::standalone::RegisterPolyfills(env_);

    TempDir dir;

    // With the encoding omitted the callback takes the second argument position.
    RunScript(
        "globalThis.error = 'unset';"
        "_fs_polyfill.readFile('" +
        dir.JsPath() + "/absent.txt', (err) => { globalThis.error = err; });");

    RunMicrotasks();
    EXPECT_TRUE(ToBool(RunScript("globalThis.error instanceof Error")));
}

TEST_F(PolyfillsTest, FsPromisesResolve) {
    dawn::node::standalone::RegisterPolyfills(env_);

    TempDir dir;
    std::string file = dir.WriteFile("hello.txt", "Hello WebGPU");

    RunScript(
        "globalThis.data = 'unset';"
        "_fs_polyfill.promises.readFile('" +
        file + "', 'utf8').then((d) => { globalThis.data = d; });");
    EXPECT_EQ(ToString(RunScript("globalThis.data")), "unset");

    RunMicrotasks();
    EXPECT_EQ(ToString(RunScript("globalThis.data")), "Hello WebGPU");

    RunScript(
        "globalThis.entries = 'unset';"
        "_fs_polyfill.promises.readdir('" +
        dir.JsPath() + "').then((e) => { globalThis.entries = e.join(','); });");
    RunMicrotasks();
    EXPECT_EQ(ToString(RunScript("globalThis.entries")), "hello.txt");

    RunScript(
        "globalThis.isFile = 'unset';"
        "_fs_polyfill.promises.stat('" +
        file + "').then((s) => { globalThis.isFile = s.isFile(); });");
    RunMicrotasks();
    EXPECT_TRUE(ToBool(RunScript("globalThis.isFile")));
}

TEST_F(PolyfillsTest, FsPromisesRejectWhenTheSyncCallThrows) {
    dawn::node::standalone::RegisterPolyfills(env_);

    TempDir dir;

    RunScript(
        "globalThis.reason = 'unset';"
        "_fs_polyfill.promises.stat('" +
        dir.JsPath() + "/absent').catch((e) => { globalThis.reason = e; });");

    RunMicrotasks();
    EXPECT_TRUE(ToBool(RunScript("globalThis.reason instanceof Error")));
}

// The path module answers in the host's separator, so the expectations below are written in their
// POSIX spelling and translated. dirname() is the exception: Node slices its answer out of the
// input, so whatever separator was written comes back unchanged.
std::string Native(std::string_view posix) {
    std::string native(posix);
#if defined(_WIN32)
    std::replace(native.begin(), native.end(), '/', '\\');
#endif
    return native;
}

// A rooted path that names no drive is resolved against the drive of the working directory, as it
// is in Node, so an absolute expectation has to carry that drive on Windows.
std::string RootedNative(std::string_view posix) {
    return std::filesystem::current_path().root_name().string() + Native(posix);
}

TEST_F(PolyfillsTest, PathNormalize) {
    dawn::node::standalone::RegisterPolyfills(env_);

    EXPECT_EQ(ToString(RunScript("_path_polyfill.normalize('/a/b/../c/./d')")), Native("/a/c/d"));
    EXPECT_EQ(ToString(RunScript("_path_polyfill.normalize('a//b')")), Native("a/b"));

    // A trailing slash says the path names a directory, so it is kept.
    EXPECT_EQ(ToString(RunScript("_path_polyfill.normalize('a//b/')")), Native("a/b/"));
    EXPECT_EQ(ToString(RunScript("_path_polyfill.normalize('/a/b/')")), Native("/a/b/"));
    EXPECT_EQ(ToString(RunScript("_path_polyfill.normalize('./')")), Native("./"));

    // The input is what decides that, and only the input: lexically_normal() invents a trailing
    // separator of its own whenever the last component was a dot segment, and it has to be
    // discarded. Without that, the first of these answers 'a/'.
    EXPECT_EQ(ToString(RunScript("_path_polyfill.normalize('a/b/..')")), Native("a"));
    EXPECT_EQ(ToString(RunScript("_path_polyfill.normalize('a/b/../')")), Native("a/"));

    // A '..' that cannot be cancelled is kept in a relative path but dropped at the root.
    EXPECT_EQ(ToString(RunScript("_path_polyfill.normalize('../../a')")), Native("../../a"));
    EXPECT_EQ(ToString(RunScript("_path_polyfill.normalize('/..')")), Native("/"));
    EXPECT_EQ(ToString(RunScript("_path_polyfill.normalize('')")), ".");

#if !defined(_WIN32)
    // A leading run of separators collapses to one. std::filesystem keeps it, because POSIX
    // reserves "//" for the implementation. POSIX-only: on Windows this shape is UNC.
    EXPECT_EQ(ToString(RunScript("_path_polyfill.normalize('//a/b')")), "/a/b");
#endif
}

TEST_F(PolyfillsTest, PathJoin) {
    dawn::node::standalone::RegisterPolyfills(env_);

    EXPECT_EQ(ToString(RunScript("_path_polyfill.join('a', 'b', 'c')")), Native("a/b/c"));
    EXPECT_EQ(ToString(RunScript("_path_polyfill.join('/a', 'b/', '../c')")), Native("/a/c"));
    EXPECT_EQ(ToString(RunScript("_path_polyfill.join('a', '', 'b')")), Native("a/b"));
    EXPECT_EQ(ToString(RunScript("_path_polyfill.join('')")), ".");

    // join() concatenates unconditionally, where operator/ would let a rooted argument discard
    // everything before it and answer '/b'.
    EXPECT_EQ(ToString(RunScript("_path_polyfill.join('/a', '/b')")), Native("/a/b"));
    EXPECT_EQ(ToString(RunScript("_path_polyfill.join('/', 'a')")), Native("/a"));

    // join() normalizes, so the trailing slash of the last argument survives, and a '..' argument
    // does not leave one behind.
    EXPECT_EQ(ToString(RunScript("_path_polyfill.join('a', 'b/')")), Native("a/b/"));
    EXPECT_EQ(ToString(RunScript("_path_polyfill.join('a', 'b', '..')")), Native("a"));
}

TEST_F(PolyfillsTest, PathDirname) {
    dawn::node::standalone::RegisterPolyfills(env_);

    // The answer is a slice of the argument, so it comes back in the separator it was written in
    // on either platform.
    EXPECT_EQ(ToString(RunScript("_path_polyfill.dirname('/a/b/c.js')")), "/a/b");
    EXPECT_EQ(ToString(RunScript("_path_polyfill.dirname('/a')")), "/");
    EXPECT_EQ(ToString(RunScript("_path_polyfill.dirname('a.js')")), ".");
    EXPECT_EQ(ToString(RunScript("_path_polyfill.dirname('')")), ".");

    // Unlike normalize(), dirname() ignores a trailing slash rather than reading it as a segment
    // boundary, so these name the parent of b, not b itself.
    EXPECT_EQ(ToString(RunScript("_path_polyfill.dirname('/a/b/')")), "/a");
    EXPECT_EQ(ToString(RunScript("_path_polyfill.dirname('a/')")), ".");

    // And it does not normalize what it keeps, which is why it cannot be parent_path(): that
    // would answer '/a'.
    EXPECT_EQ(ToString(RunScript("_path_polyfill.dirname('/a//b')")), "/a/");

#if !defined(_WIN32)
    // Nothing but separators names the root, and a leading "//" is kept whole. POSIX-only: on
    // Windows two leading separators are UNC, which is refused rather than answered.
    EXPECT_EQ(ToString(RunScript("_path_polyfill.dirname('///')")), "/");
    EXPECT_EQ(ToString(RunScript("_path_polyfill.dirname('//a')")), "//");
#endif
}

TEST_F(PolyfillsTest, PathResolve) {
    dawn::node::standalone::RegisterPolyfills(env_);

    EXPECT_EQ(ToString(RunScript("_path_polyfill.resolve('/a/b', 'c')")), RootedNative("/a/b/c"));
    EXPECT_EQ(ToString(RunScript("_path_polyfill.resolve('/a/b', '../c')")), RootedNative("/a/c"));

    // An absolute argument discards everything to its left, and a call with no absolute argument
    // at all is taken relative to the working directory.
    EXPECT_EQ(ToString(RunScript("_path_polyfill.resolve('/a/b', '/c')")), RootedNative("/c"));
    EXPECT_EQ(ToString(RunScript("_path_polyfill.resolve('a')")),
              (std::filesystem::current_path() / "a").string());

    // Unlike normalize(), resolve() never answers with a trailing separator, whether the argument
    // carried one or lexically_normal() invented one.
    EXPECT_EQ(ToString(RunScript("_path_polyfill.resolve('/a/b/')")), RootedNative("/a/b"));
    EXPECT_EQ(ToString(RunScript("_path_polyfill.resolve('/a/b/..')")), RootedNative("/a"));

    // An empty argument is skipped rather than read as '.', and a call with nothing left is the
    // working directory. std::filesystem::absolute('') fails outright.
    EXPECT_EQ(ToString(RunScript("_path_polyfill.resolve('/a', '')")), RootedNative("/a"));
    EXPECT_EQ(ToString(RunScript("_path_polyfill.resolve('')")),
              std::filesystem::current_path().string());
}

TEST_F(PolyfillsTest, PathRelative) {
    dawn::node::standalone::RegisterPolyfills(env_);

    EXPECT_EQ(ToString(RunScript("_path_polyfill.relative('/a/b', '/a/b/c/d')")), Native("c/d"));

    // Identical paths are "" and not ".", which is what lexically_relative() answers.
    EXPECT_EQ(ToString(RunScript("_path_polyfill.relative('/a/b', '/a/b')")), "");

    // Leaving the common prefix behind takes one '..' per remaining segment of `from`.
    EXPECT_EQ(ToString(RunScript("_path_polyfill.relative('/a/b/c', '/a/d')")), Native("../../d"));

    // A zero-length side means the working directory, because resolve() is what defines both ends.
    EXPECT_EQ(ToString(RunScript("_path_polyfill.relative('', 'a')")), "a");
}

TEST_F(PolyfillsTest, PathRejectsUncPaths) {
    dawn::node::standalone::RegisterPolyfills(env_);

    // The same refusal the fs entry points make, for the same reason: Node and std::filesystem
    // disagree about where a UNC root ends. Any argument can carry the prefix, not just the first.
    EXPECT_TRUE(Throws(R"(_path_polyfill.normalize(String.raw`\\server\share\a`))"));
    EXPECT_TRUE(Throws(R"(_path_polyfill.dirname(String.raw`\\server\share\a`))"));
    EXPECT_TRUE(Throws(R"(_path_polyfill.join('a', String.raw`\\server\share`))"));
    EXPECT_TRUE(Throws(R"(_path_polyfill.resolve(String.raw`\\server\share`))"));
    EXPECT_TRUE(Throws(R"(_path_polyfill.relative('a', String.raw`\\?\C:\a`))"));
}

#if defined(_WIN32)
// The shapes that have no POSIX spelling to translate. Every expectation here is what
// path.win32 answers.
TEST_F(PolyfillsTest, PathDriveLetters) {
    dawn::node::standalone::RegisterPolyfills(env_);

    // A drive with no root directory names the working directory on that drive, and Node spells
    // the implied here-ness out rather than leaving the bare drive.
    EXPECT_EQ(ToString(RunScript("_path_polyfill.normalize('C:')")), "C:.");
    EXPECT_EQ(ToString(RunScript("_path_polyfill.normalize('C:.')")), "C:.");

    // Either separator is accepted on the way in; the preferred one comes back out.
    EXPECT_EQ(ToString(RunScript("_path_polyfill.normalize('C:/a/b/')")), "C:\\a\\b\\");
    EXPECT_EQ(ToString(RunScript("_path_polyfill.normalize('C:/a/b/..')")), "C:\\a");
    EXPECT_EQ(ToString(RunScript("_path_polyfill.join('C:/a', 'b/')")), "C:\\a\\b\\");
    EXPECT_EQ(ToString(RunScript("_path_polyfill.resolve('C:/a', 'b')")), "C:\\a\\b");
    EXPECT_EQ(ToString(RunScript("_path_polyfill.relative('C:/a/b', 'C:/a/c')")), "..\\c");

    // Except from dirname(), which slices the argument and so keeps what was written. The drive
    // is the root here, and is never cut into.
    EXPECT_EQ(ToString(RunScript(R"(_path_polyfill.dirname(String.raw`C:\a\b`))")), "C:\\a");
    EXPECT_EQ(ToString(RunScript("_path_polyfill.dirname('C:/a//b')")), "C:/a/");
    EXPECT_EQ(ToString(RunScript(R"(_path_polyfill.dirname(String.raw`C:\a`))")), "C:\\");

    // Two drives are unrelated, so there is no path from one to the other and the answer is just
    // the destination. lexically_relative() says "" for that, which would read as "the same
    // place". Purely lexical, so the drives need not exist.
    EXPECT_EQ(ToString(RunScript("_path_polyfill.relative('C:/a', 'D:/b')")), "D:\\b");
}
#endif

TEST_F(PolyfillsTest, PathRejectsArgumentsThatAreNotStrings) {
    dawn::node::standalone::RegisterPolyfills(env_);

    EXPECT_TRUE(Throws("_path_polyfill.normalize()"));
    EXPECT_TRUE(Throws("_path_polyfill.normalize(5)"));
    EXPECT_TRUE(Throws("_path_polyfill.join('a', null)"));
    EXPECT_TRUE(Throws("_path_polyfill.relative('a')"));
}

TEST_F(PolyfillsTest, PathSep) {
    dawn::node::standalone::RegisterPolyfills(env_);

    EXPECT_EQ(ToString(RunScript("_path_polyfill.sep")), Native("/"));
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
