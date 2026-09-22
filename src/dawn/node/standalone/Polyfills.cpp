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

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

#include "src/dawn/common/SystemUtils.h"

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
// Paths
// ---------------------------------------------------------------------------

// Whether `c` can begin a UNC path on this platform.
//
// Windows accepts both "\\server\share" and "//server/share" as UNC. On POSIX only the backslash
// form is treated as one, because a leading "//" is a legal POSIX path.
bool BeginsUncPath(char c) {
#if defined(_WIN32)
    return c == '/' || c == '\\';
#else
    return c == '\\';
#endif
}

// Returns true if the path begins with two separators. That is the whole test: anything beginning
// that way is a UNC path ("\\server\share") or a device path ("\\?\C:\..." and "\\.\...").
bool IsUncPath(std::string_view path) {
    return path.size() >= 2 && BeginsUncPath(path[0]) && BeginsUncPath(path[1]);
}

// Throws a JavaScript exception if `path` is a UNC or device path. Returns true if it threw.
//
// These are deliberately unsupported. Node.js handles them very differently
// than std::filesystem::path. To match Node.js, we would need to implement it
// from scratch. We will not do that unless there is a request from the users.
bool RejectUncPath(Napi::Env env, const std::string& path) {
    if (!IsUncPath(path)) {
        return false;
    }
    Napi::Error::New(env, "UNC and device paths are not supported: " + path)
        .ThrowAsJavaScriptException();
    return true;
}

// ---------------------------------------------------------------------------
// fs
// ---------------------------------------------------------------------------

// Retrieves the path argument from `info` and stores it in `*path`. Every fs
// entry point with a path argument has it in the first position. If the path
// cannot be retrieved or it is a UNC path, a JavaScript exception is thrown.
bool GetPathArgument(const Napi::CallbackInfo& info, std::string* path) {
    if (info.Length() < 1 || !info[0].IsString()) {
        Napi::TypeError::New(info.Env(), "String expected for path").ThrowAsJavaScriptException();
        return false;
    }
    *path = info[0].As<Napi::String>().Utf8Value();
    return !RejectUncPath(info.Env(), *path);
}

// Which encoding the caller asked for. What kNone means is up to the caller: readFileSync()
// answers with bytes, readdirSync() with UTF-8 strings, matching Node's defaults.
enum class Encoding {
    kNone,
    kUtf8,
};

// Reads the options argument the fs entry points take, which may be an encoding on its own or an
// object holding one.
//
// Only UTF-8 is implemented and `encoding` is the only option understood. Anything else throws a
// JavaScript exception.
bool GetEncodingOption(const Napi::CallbackInfo& info, Encoding* encoding) {
    Napi::Env env = info.Env();
    *encoding = Encoding::kNone;
    if (info.Length() < 2 || info[1].IsUndefined() || info[1].IsNull()) {
        return true;
    }

    Napi::Value value = info[1];
    if (value.IsObject()) {
        Napi::Object options = value.As<Napi::Object>();
        Napi::Array names = options.GetPropertyNames();
        for (uint32_t i = 0; i < names.Length(); ++i) {
            std::string name = names.Get(i).ToString().Utf8Value();
            if (name != "encoding") {
                Napi::Error::New(env, "Unsupported fs option: " + name)
                    .ThrowAsJavaScriptException();
                return false;
            }
        }
        value = options.Get("encoding");
        if (value.IsUndefined() || value.IsNull()) {
            return true;
        }
    }

    if (!value.IsString()) {
        Napi::TypeError::New(env, "String expected for encoding").ThrowAsJavaScriptException();
        return false;
    }
    std::string name = value.As<Napi::String>().Utf8Value();
    std::string lower_name = name;
    for (char& c : lower_name) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    // Node.js documentation specifies that all buffer encodings are case-insensitive.
    if (lower_name != "utf8" && lower_name != "utf-8") {
        Napi::Error::New(env, "Unsupported fs encoding: " + name).ThrowAsJavaScriptException();
        return false;
    }
    *encoding = Encoding::kUtf8;
    return true;
}

Napi::Value ReadFileSync(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    std::string path;
    Encoding encoding = Encoding::kNone;
    if (!GetPathArgument(info, &path) || !GetEncodingOption(info, &encoding)) {
        return env.Undefined();
    }

    // Node.js reads the file in binary regardless of the encoding.
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        Napi::Error::New(env, "Failed to open file: " + path).ThrowAsJavaScriptException();
        return env.Undefined();
    }

    const std::streamoff offset = file.tellg();
    if (offset < 0) {
        Napi::Error::New(env, "Failed to size file: " + path).ThrowAsJavaScriptException();
        return env.Undefined();
    }
    const size_t size = static_cast<size_t>(offset);

    file.seekg(0, std::ios::beg);

    // Read straight into the object being returned, so the contents are written once.
    // Resize to gcount() in case the file shrank between the seek and the read.
    if (encoding == Encoding::kUtf8) {
        std::string content(size, '\0');
        file.read(content.data(), offset);
        content.resize(static_cast<size_t>(file.gcount()));
        return Napi::String::New(env, content);
    }
    Napi::ArrayBuffer array_buffer = Napi::ArrayBuffer::New(env, size);
    file.read(static_cast<char*>(array_buffer.Data()), offset);
    return Napi::Uint8Array::New(env, static_cast<size_t>(file.gcount()), array_buffer, 0);
}

// Returns true if the given file exists. A JavaScript exception is thrown if a
// UNC path is given.
Napi::Value ExistsSync(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 1 || !info[0].IsString()) {
        return Napi::Boolean::New(env, false);
    }
    std::string path = info[0].As<Napi::String>().Utf8Value();
    if (RejectUncPath(env, path)) {
        return env.Undefined();
    }
    std::error_code ec;
    bool exists = std::filesystem::exists(path, ec);
    return Napi::Boolean::New(env, exists && !ec);
}

Napi::Value ReaddirSync(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    std::string path;
    // The entries are always strings, so the encoding needs validating rather than honouring; the
    // point of the call is to reject `withFileTypes` and friends.
    Encoding encoding = Encoding::kNone;
    if (!GetPathArgument(info, &path) || !GetEncodingOption(info, &encoding)) {
        return env.Undefined();
    }

    std::error_code ec;
    std::filesystem::directory_iterator it(path, ec);
    if (ec) {
        Napi::Error::New(env, ec.message()).ThrowAsJavaScriptException();
        return env.Undefined();
    }

    Napi::Array result = Napi::Array::New(env);
    uint32_t index = 0;
    for (const auto& entry : it) {
        result.Set(index++, Napi::String::New(env, entry.path().filename().string()));
    }
    return result;
}

Napi::Value ReturnTrue(const Napi::CallbackInfo& info) {
    return Napi::Boolean::New(info.Env(), true);
}

Napi::Value ReturnFalse(const Napi::CallbackInfo& info) {
    return Napi::Boolean::New(info.Env(), false);
}

Napi::Value StatSync(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    std::string path;
    Encoding encoding = Encoding::kNone;
    if (!GetPathArgument(info, &path) || !GetEncodingOption(info, &encoding)) {
        return env.Undefined();
    }

    std::error_code ec;
    std::filesystem::file_status status = std::filesystem::status(path, ec);
    if (ec || !std::filesystem::exists(status)) {
        Napi::Error::New(env, "Failed to stat: " + path).ThrowAsJavaScriptException();
        return env.Undefined();
    }

    // The answer is settled here, so each predicate is just the constant it will always return.
    Napi::Object stats = Napi::Object::New(env);
    stats.Set(
        "isFile",
        Napi::Function::New(
            env, std::filesystem::is_regular_file(status) ? ReturnTrue : ReturnFalse, "isFile"));
    stats.Set(
        "isDirectory",
        Napi::Function::New(env, std::filesystem::is_directory(status) ? ReturnTrue : ReturnFalse,
                            "isDirectory"));
    return stats;
}

// https://nodejs.org/api/fs.html
//
// Only the synchronous entry points are native. The callback and promise forms are built on top of
// them by the bootstrap script.
void RegisterFs(Napi::Env env) {
    Napi::Object fs = Napi::Object::New(env);
    fs.Set("readFileSync", Napi::Function::New(env, ReadFileSync, "readFileSync"));
    fs.Set("existsSync", Napi::Function::New(env, ExistsSync, "existsSync"));
    fs.Set("readdirSync", Napi::Function::New(env, ReaddirSync, "readdirSync"));
    fs.Set("statSync", Napi::Function::New(env, StatSync, "statSync"));
    env.Global().Set("_fs_polyfill", fs);
}

// ---------------------------------------------------------------------------
// path
// ---------------------------------------------------------------------------
//
// Built on std::filesystem, which walks the components and, on Windows, understands drive letters
// and accepts either separator. Its conventions are not Node's, so each function below lets
// std::filesystem handle the general case and then fixes up the places where the two disagree.
//
// UNC and device paths are refused rather than reconciled; see RejectUncPath.

constexpr char kPreferredSeparator = static_cast<char>(std::filesystem::path::preferred_separator);

// What counts as a separator on this host. Windows accepts either; POSIX only the forward slash.
#if defined(_WIN32)
constexpr std::string_view kSeparators = "/\\";
#else
constexpr std::string_view kSeparators = "/";
#endif

bool IsSeparator(char c) {
    return kSeparators.find(c) != std::string_view::npos;
}

// A trailing separator is a flag to Node but a real, empty final component to std::filesystem, so
// filename(), extension() and parent_path() all see one component more than Node does. Dropping it
// first is what makes them agree. The root is never stripped: "/" and "C:\" are not trailing
// separators.
std::string TrimTrailingSeparators(const std::filesystem::path& path) {
    const std::string text = path.string();
    const size_t root = path.root_path().string().size();
    const size_t last = text.find_last_not_of(kSeparators);
    const size_t end = last == std::string::npos ? root : std::max(root, last + 1);
    return text.substr(0, end);
}

// Two or more leading separators collapse to one, which is what Node does on POSIX. On Windows two
// separators would mean UNC, and those are refused at the boundary, so anything arriving here with
// the shape is an ordinary rooted path that concatenation happened to produce - join('/', 'a')
// builds "/\a" on the way to "\a".
std::string CollapseLeadingSeparators(std::string text) {
    size_t run = text.find_first_not_of(kSeparators);
    if (run == std::string::npos) {
        run = text.size();  // nothing but separators
    }
    if (run > 1) {
        text.erase(0, run - 1);
    }
    return text;
}

std::string NormalizePath(const std::string& input) {
    if (input.empty()) {
        return ".";  // lexically_normal() returns ""
    }
    const std::filesystem::path path(input);
    // lexically_normal() both drops a trailing separator ("./" becomes ".") and invents one
    // ("a/b/.." becomes "a/"), so neither its presence nor its absence in the output means
    // anything. Taking it off and putting it back according to the input is what Node does.
    const bool trailing = IsSeparator(input.back());
    std::string result = TrimTrailingSeparators(path.lexically_normal());
    if (result.empty()) {
        result = ".";
    } else if (path.has_root_name() && !path.has_root_directory() &&
               result == path.root_name().string()) {
        // A path like "C:" on Windows implies the "." directory on that drive.
        // Node makes this explicit by turning it into "C:.".
        // This cannot happen for POSIX.
        result += '.';
    }
    if (trailing && !IsSeparator(result.back())) {
        result += kPreferredSeparator;
    }
    return CollapseLeadingSeparators(result);
}

// The one entry point not expressible in terms of a std::filesystem primitive. Node's dirname() is
// a raw string slice that deliberately does not normalize - dirname("/a//b") is "/a/", separator
// run intact - while parent_path() is component-based and has already discarded that. So Node's
// scan is transcribed, with root_path() supplying the one genuinely platform-specific part: where
// the root ends, be that "/" or "C:\".
std::string DirnamePath(const std::string& input) {
    if (input.empty()) {
        return ".";
    }
    const size_t root_length = std::filesystem::path(input).root_path().string().size();
    const bool has_root = root_length > 0;

    // Search for the last separator, ignoring any trailing separators and the
    // root. Separators in the root have a special meaning. The result is
    // everything that appears before the separator preceding the last component.
    const std::string_view path_without_root = std::string_view(input).substr(root_length);

    // Skip any trailing separators to locate the end of the last path component.
    const size_t last_non_separator = path_without_root.find_last_not_of(kSeparators);

    // Find the separator immediately preceding the last component.
    const size_t separator_before_last_component =
        last_non_separator == std::string_view::npos
            ? std::string_view::npos
            : path_without_root.find_last_of(kSeparators, last_non_separator);
    if (separator_before_last_component == std::string_view::npos) {
        // Nothing to cut back to, so the answer is the root, or "." when there is no root.
        return has_root ? input.substr(0, root_length) : ".";
    }

    // Find the index of the separator in the original string.
    const size_t last_separator_index = root_length + separator_before_last_component;

    /*
     * Node's posix dirname() has exactly one special case, quoted from Node's implementation:
     *
     *   // POSIX reserves a leading '//' for implementation-defined purposes.
     *   // (IEEE Std 1003.1-2017, Section 4.13 Pathname Resolution:
     *   //  "A pathname that begins with two successive slashes may be interpreted
     *   //   in an implementation-defined manner, although more than two leading
     *   //   slashes shall be treated as a single slash.")
     *   // Node keeps both slashes: dirname("//a") is "//" rather than "/".
     *
     * On Windows, leading double slashes represent a UNC path, which is rejected
     * earlier before reaching this function.
     */
    if (has_root && last_separator_index == 1) {
        return input.substr(0, 2);
    }

    // Return the substring up to, but not including, the separator before the last component.
    return input.substr(0, last_separator_index);
}

std::string JoinPaths(const std::vector<std::string>& args) {
    // std::filesystem::path::append cannot be used here. If one of `args` is an
    // absolute path, Node will join them: {"/a", "/b"} -> "/a/b". However,
    // std::filesystem::path drops the previous paths: {"/a", "/b"} -> "/b".
    std::string joined;
    for (const std::string& arg : args) {
        if (arg.empty()) {
            continue;  // Node skips empty arguments rather than reading them as "."
        }
        if (!joined.empty()) {
            joined += kPreferredSeparator;
        }
        joined += arg;
    }
    if (joined.empty()) {
        return ".";
    }
    return NormalizePath(joined);
}

// The working directory, or false with an exception pending. Shared by process.cwd() and by
// resolve(), which anchors relative arguments on it.
bool CurrentDirectory(Napi::Env env, std::string* out) {
    std::error_code ec;
    const std::filesystem::path cwd = std::filesystem::current_path(ec);
    if (ec) {
        Napi::Error::New(env, "Could not read the working directory: " + ec.message())
            .ThrowAsJavaScriptException();
        return false;
    }
    *out = cwd.string();
    // A UNC working directory means a checkout on a network share. Everything resolve() produces
    // would be built on it, so it fails here rather than somewhere downstream that gives no hint
    // of the cause.
    return !RejectUncPath(env, *out);
}

bool ResolvePaths(Napi::Env env, const std::vector<std::string>& args, std::string* out) {
    std::filesystem::path accumulated;
    for (const std::string& arg : args) {
        if (arg.empty()) {
            continue;  // absolute("") fails with EINVAL, and Node ignores empty arguments
        }
        // Here operator/ is exactly right: its rule that a later absolute argument replaces
        // everything before it, including the Windows rule that a rooted argument keeps the
        // left-hand side's drive, is what resolve() specifies.
        accumulated /= std::filesystem::path(arg);
    }

    // The working directory is read here rather than left to absolute() so that a working
    // directory that cannot be read, or that is UNC, is reported as such instead of surfacing as a
    // confusing failure about the argument.
    if (!accumulated.is_absolute()) {
        std::string cwd;
        if (!CurrentDirectory(env, &cwd)) {
            return false;
        }

        // If `accumulated` is empty, the call to std::filesystem::absolute
        // below will fail. Replacing it with `cwd` to match Node's behaviour.
        if (accumulated.empty()) {
            accumulated = std::filesystem::path(cwd);
        }
    }

    // absolute() rather than `cwd / accumulated`: on Windows a drive-relative argument like "C:a"
    // names the working directory of that drive, which only absolute() knows how to consult -
    // operator/ would see a root-name of its own and keep "C:a" relative. On POSIX the two are the
    // same thing.
    std::error_code ec;
    const std::filesystem::path absolute = std::filesystem::absolute(accumulated, ec);
    if (ec) {
        Napi::Error::New(env, "Could not resolve path: " + ec.message())
            .ThrowAsJavaScriptException();
        return false;
    }
    // Node's resolve() never returns a trailing separator, but lexically_normal() produces one
    // whenever the last component was a dot segment.
    std::string result = TrimTrailingSeparators(absolute.lexically_normal());
    if (result.empty()) {
        result = ".";
    }
    *out = CollapseLeadingSeparators(result);
    return true;
}

bool RelativePath(Napi::Env env, const std::string& from, const std::string& to, std::string* out) {
    // Node resolves both sides first, so the answer depends only on where they land.
    std::string from_resolved;
    std::string to_resolved;
    if (!ResolvePaths(env, {from}, &from_resolved) || !ResolvePaths(env, {to}, &to_resolved)) {
        return false;
    }

    const std::filesystem::path result =
        std::filesystem::path(to_resolved).lexically_relative(std::filesystem::path(from_resolved));
    if (result == std::filesystem::path(".")) {
        *out = "";  // lexically_relative() says "." for identical paths; Node says ""
    } else if (result.empty()) {
        *out = to_resolved;  // unrelated roots, where Node falls back to the resolved `to`
    } else {
        *out = result.string();
    }
    return true;
}

// Reads the string arguments a path entry point takes, throwing the TypeError Node throws for a
// non-string, and refusing UNC paths.
bool GetPathArguments(const Napi::CallbackInfo& info,
                      size_t least,
                      std::vector<std::string>* args) {
    if (info.Length() < least) {
        Napi::TypeError::New(info.Env(), "String expected for path").ThrowAsJavaScriptException();
        return false;
    }
    for (size_t i = 0; i < info.Length(); ++i) {
        if (!info[i].IsString()) {
            Napi::TypeError::New(info.Env(), "String expected for path")
                .ThrowAsJavaScriptException();
            return false;
        }
        std::string arg = info[i].As<Napi::String>().Utf8Value();
        if (RejectUncPath(info.Env(), arg)) {
            return false;
        }
        args->push_back(std::move(arg));
    }
    return true;
}

Napi::Value PathNormalize(const Napi::CallbackInfo& info) {
    std::vector<std::string> args;
    if (!GetPathArguments(info, 1, &args)) {
        return info.Env().Undefined();
    }
    return Napi::String::New(info.Env(), NormalizePath(args[0]));
}

Napi::Value PathDirname(const Napi::CallbackInfo& info) {
    std::vector<std::string> args;
    if (!GetPathArguments(info, 1, &args)) {
        return info.Env().Undefined();
    }
    return Napi::String::New(info.Env(), DirnamePath(args[0]));
}

Napi::Value PathJoin(const Napi::CallbackInfo& info) {
    std::vector<std::string> args;
    if (!GetPathArguments(info, 0, &args)) {
        return info.Env().Undefined();
    }
    return Napi::String::New(info.Env(), JoinPaths(args));
}

Napi::Value PathResolve(const Napi::CallbackInfo& info) {
    std::vector<std::string> args;
    std::string resolved;
    if (!GetPathArguments(info, 0, &args) || !ResolvePaths(info.Env(), args, &resolved)) {
        return info.Env().Undefined();
    }
    return Napi::String::New(info.Env(), resolved);
}

Napi::Value PathRelative(const Napi::CallbackInfo& info) {
    std::vector<std::string> args;
    std::string relative;
    if (!GetPathArguments(info, 2, &args) ||
        !RelativePath(info.Env(), args[0], args[1], &relative)) {
        return info.Env().Undefined();
    }
    return Napi::String::New(info.Env(), relative);
}

// https://nodejs.org/api/path.html
void RegisterPath(Napi::Env env) {
    Napi::Object path = Napi::Object::New(env);
    path.Set("dirname", Napi::Function::New(env, PathDirname, "dirname"));
    path.Set("join", Napi::Function::New(env, PathJoin, "join"));
    path.Set("normalize", Napi::Function::New(env, PathNormalize, "normalize"));
    path.Set("relative", Napi::Function::New(env, PathRelative, "relative"));
    path.Set("resolve", Napi::Function::New(env, PathResolve, "resolve"));
    path.Set("sep", Napi::String::New(env, std::string(1, kPreferredSeparator)));
    env.Global().Set("_path_polyfill", path);
}

// ---------------------------------------------------------------------------
// process
// ---------------------------------------------------------------------------

Napi::Value Cwd(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    // The working directory is the one path the runtime is not handed by its caller, so it is also
    // the one that can go wrong without anybody having asked for anything unusual. Everything
    // resolve() produces is built on it, so CurrentDirectory() throws rather than return a string
    // that would quietly contaminate every path derived from it.
    std::string cwd;
    if (!CurrentDirectory(env, &cwd)) {
        return env.Undefined();
    }
    return Napi::String::New(env, cwd);
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
    if (auto [dawn_flags, is_set] = dawn::GetEnvironmentVar("DAWN_FLAGS"); is_set) {
        env_obj.Set("DAWN_FLAGS", Napi::String::New(env, dawn_flags));
    }
    process.Set("env", env_obj);

    Napi::Array argv_array = Napi::Array::New(env, options.argv.size());
    for (uint32_t i = 0; i < options.argv.size(); ++i) {
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
    // The callback and promise forms of fs, over the native synchronous calls. Deferring to a
    // microtask is what makes them asynchronous; the work itself still blocks.
    // https://nodejs.org/api/fs.html
    const fs = globalThis._fs_polyfill;

    fs.readFile = function(path, options, callback) {
        if (typeof options === 'function') {
            callback = options;
            options = undefined;
        }
        Promise.resolve().then(() => {
            let data;
            try {
                data = fs.readFileSync(path, options);
            } catch (err) {
                callback(err);
                return;
            }
            callback(null, data);
        });
    };

    fs.promises = {
        readdir: (path) => Promise.resolve().then(() => fs.readdirSync(path)),
        stat: (path) => Promise.resolve().then(() => fs.statSync(path)),
        readFile: (path, options) => Promise.resolve().then(() => fs.readFileSync(path, options)),
    };
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
    RegisterFs(env);
    RegisterPath(env);
    RegisterProcess(env, options, ctx);
    RegisterPerformance(env, ctx);
    RunBootstrapScript(env);
}

}  // namespace dawn::node::standalone
