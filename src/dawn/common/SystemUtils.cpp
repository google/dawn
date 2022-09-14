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

#include "dawn/common/SystemUtils.h"

#include "dawn/common/Assert.h"
#include "dawn/common/Log.h"

#if DAWN_PLATFORM_IS(WINDOWS)
#include <Windows.h>
#include <vector>
#elif DAWN_PLATFORM_IS(LINUX)
#include <dlfcn.h>
#include <limits.h>
#include <unistd.h>
#include <cstdlib>
#elif DAWN_PLATFORM_IS(MACOS) || DAWN_PLATFORM_IS(IOS)
#include <dlfcn.h>
#include <mach-o/dyld.h>
#include <vector>
#endif

#include <array>

#if DAWN_PLATFORM_IS(WINDOWS)
const char* GetPathSeparator() {
    return "\\";
}

std::pair<std::string, bool> GetEnvironmentVar(const char* variableName) {
    // First pass a size of 0 to get the size of variable value.
    DWORD sizeWithNullTerminator = GetEnvironmentVariableA(variableName, nullptr, 0);
    if (sizeWithNullTerminator == 0) {
        DWORD err = GetLastError();
        if (err != ERROR_ENVVAR_NOT_FOUND) {
            dawn::WarningLog() << "GetEnvironmentVariableA failed with code " << err;
        }
        return std::make_pair(std::string(), false);
    }

    // Then get variable value with its actual size.
    std::vector<char> buffer(sizeWithNullTerminator);
    DWORD sizeStored =
        GetEnvironmentVariableA(variableName, buffer.data(), static_cast<DWORD>(buffer.size()));
    if (sizeStored + 1 != sizeWithNullTerminator) {
        DWORD err = GetLastError();
        if (err) {
            dawn::WarningLog() << "GetEnvironmentVariableA failed with code " << err;
        }
        return std::make_pair(std::string(), false);
    }
    return std::make_pair(std::string(buffer.data(), sizeStored), true);
}

bool SetEnvironmentVar(const char* variableName, const char* value) {
    return SetEnvironmentVariableA(variableName, value) == TRUE;
}
#elif DAWN_PLATFORM_IS(POSIX)
const char* GetPathSeparator() {
    return "/";
}

std::pair<std::string, bool> GetEnvironmentVar(const char* variableName) {
    char* value = getenv(variableName);
    return value == nullptr ? std::make_pair(std::string(), false)
                            : std::make_pair(std::string(value), true);
}

bool SetEnvironmentVar(const char* variableName, const char* value) {
    if (value == nullptr) {
        return unsetenv(variableName) == 0;
    }
    return setenv(variableName, value, 1) == 0;
}
#else
#error "Implement Get/SetEnvironmentVar for your platform."
#endif

#if DAWN_PLATFORM_IS(WINDOWS)
std::optional<std::string> GetHModulePath(HMODULE module) {
    std::array<char, MAX_PATH> executableFileBuf;
    DWORD executablePathLen = GetModuleFileNameA(nullptr, executableFileBuf.data(),
                                                 static_cast<DWORD>(executableFileBuf.size()));
    if (executablePathLen == 0) {
        return {};
    }
    return executableFileBuf.data();
}
std::optional<std::string> GetExecutablePath() {
    return GetHModulePath(nullptr);
}
#elif DAWN_PLATFORM_IS(LINUX)
std::optional<std::string> GetExecutablePath() {
    std::array<char, PATH_MAX> path;
    ssize_t result = readlink("/proc/self/exe", path.data(), PATH_MAX - 1);
    if (result < 0 || static_cast<size_t>(result) >= PATH_MAX - 1) {
        return {};
    }

    path[result] = '\0';
    return path.data();
}
#elif DAWN_PLATFORM_IS(MACOS) || DAWN_PLATFORM_IS(IOS)
std::optional<std::string> GetExecutablePath() {
    uint32_t size = 0;
    _NSGetExecutablePath(nullptr, &size);

    std::vector<char> buffer(size + 1);
    if (_NSGetExecutablePath(buffer.data(), &size) != 0) {
        return {};
    }

    buffer[size] = '\0';
    return buffer.data();
}
#elif DAWN_PLATFORM_IS(FUCHSIA)
std::optional<std::string> GetExecutablePath() {
    // UNIMPLEMENTED
    return {};
}
#elif DAWN_PLATFORM_IS(EMSCRIPTEN)
std::optional<std::string> GetExecutablePath() {
    return {};
}
#else
#error "Implement GetExecutablePath for your platform."
#endif

std::optional<std::string> GetExecutableDirectory() {
    std::optional<std::string> exePath = GetExecutablePath();
    if (!exePath) {
        return {};
    }
    size_t lastPathSepLoc = exePath->find_last_of(GetPathSeparator());
    if (lastPathSepLoc == std::string::npos) {
        return {};
    }
    return exePath->substr(0, lastPathSepLoc + 1);
}

#if DAWN_PLATFORM_IS(LINUX) || DAWN_PLATFORM_IS(MACOS) || DAWN_PLATFORM_IS(IOS)
std::optional<std::string> GetModulePath() {
    static int placeholderSymbol = 0;
    Dl_info dlInfo;
    if (dladdr(&placeholderSymbol, &dlInfo) == 0) {
        return {};
    }

    std::array<char, PATH_MAX> absolutePath;
    if (realpath(dlInfo.dli_fname, absolutePath.data()) == nullptr) {
        return {};
    }
    return absolutePath.data();
}
#elif DAWN_PLATFORM_IS(WINDOWS)
std::optional<std::string> GetModulePath() {
    static int placeholderSymbol = 0;
    HMODULE module = nullptr;
// GetModuleHandleEx is unavailable on UWP
#if defined(DAWN_IS_WINUWP)
    return {};
#else
    if (!GetModuleHandleExA(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            reinterpret_cast<LPCSTR>(&placeholderSymbol), &module)) {
        return {};
    }
#endif
    return GetHModulePath(module);
}
#elif DAWN_PLATFORM_IS(FUCHSIA)
std::optional<std::string> GetModulePath() {
    return {};
}
#elif DAWN_PLATFORM_IS(EMSCRIPTEN)
std::optional<std::string> GetModulePath() {
    return {};
}
#else
#error "Implement GetModulePath for your platform."
#endif

std::optional<std::string> GetModuleDirectory() {
    std::optional<std::string> modPath = GetModulePath();
    if (!modPath) {
        return {};
    }
    size_t lastPathSepLoc = modPath->find_last_of(GetPathSeparator());
    if (lastPathSepLoc == std::string::npos) {
        return {};
    }
    return modPath->substr(0, lastPathSepLoc + 1);
}

// ScopedEnvironmentVar

ScopedEnvironmentVar::ScopedEnvironmentVar() = default;

ScopedEnvironmentVar::ScopedEnvironmentVar(const char* variableName, const char* value)
    : mName(variableName),
      mOriginalValue(GetEnvironmentVar(variableName)),
      mIsSet(SetEnvironmentVar(variableName, value)) {}

ScopedEnvironmentVar::~ScopedEnvironmentVar() {
    if (mIsSet) {
        bool success = SetEnvironmentVar(
            mName.c_str(), mOriginalValue.second ? mOriginalValue.first.c_str() : nullptr);
        // If we set the environment variable in the constructor, we should never fail restoring it.
        ASSERT(success);
    }
}

bool ScopedEnvironmentVar::Set(const char* variableName, const char* value) {
    ASSERT(!mIsSet);
    mName = variableName;
    mOriginalValue = GetEnvironmentVar(variableName);
    mIsSet = SetEnvironmentVar(variableName, value);
    return mIsSet;
}
