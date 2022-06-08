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

#ifndef SRC_DAWN_COMMON_SYSTEMUTILS_H_
#define SRC_DAWN_COMMON_SYSTEMUTILS_H_

#include <optional>
#include <string>
#include <utility>

#include "dawn/common/Platform.h"

const char* GetPathSeparator();
// Returns a pair of the environment variable's value, and a boolean indicating whether the variable
// was present.
std::pair<std::string, bool> GetEnvironmentVar(const char* variableName);
bool SetEnvironmentVar(const char* variableName, const char* value);
// Directories are always returned with a trailing path separator.
// May return std::nullopt if the path is too long, there is no current
// module (statically linked into executable), or the function is not
// implemented on the platform.
std::optional<std::string> GetExecutableDirectory();
std::optional<std::string> GetModuleDirectory();

#if DAWN_PLATFORM_IS(MACOS)
void GetMacOSVersion(int32_t* majorVersion, int32_t* minorVersion = nullptr);
bool IsMacOSVersionAtLeast(uint32_t majorVersion, uint32_t minorVersion = 0);
#endif

class ScopedEnvironmentVar {
  public:
    ScopedEnvironmentVar();
    ScopedEnvironmentVar(const char* variableName, const char* value);
    ~ScopedEnvironmentVar();

    ScopedEnvironmentVar(const ScopedEnvironmentVar& rhs) = delete;
    ScopedEnvironmentVar& operator=(const ScopedEnvironmentVar& rhs) = delete;

    bool Set(const char* variableName, const char* value);

  private:
    std::string mName;
    std::pair<std::string, bool> mOriginalValue;
    bool mIsSet = false;
};

#endif  // SRC_DAWN_COMMON_SYSTEMUTILS_H_
