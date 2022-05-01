// Copyright 2022 The Tint Authors.
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

#include "src/tint/utils/debugger.h"

#ifdef TINT_ENABLE_BREAK_IN_DEBUGGER

#ifdef _MSC_VER
#include <Windows.h>
#elif defined(__linux__)
#include <signal.h>
#include <fstream>
#include <string>
#endif

#ifdef _MSC_VER
#define TINT_DEBUGGER_BREAK_DEFINED
void tint::debugger::Break() {
    if (::IsDebuggerPresent()) {
        ::DebugBreak();
    }
}

#elif defined(__linux__)

#define TINT_DEBUGGER_BREAK_DEFINED
void tint::debugger::Break() {
    // A process is being traced (debugged) if "/proc/self/status" contains a
    // line with "TracerPid: <non-zero-digit>...".
    bool is_traced = false;
    std::ifstream fin("/proc/self/status");
    std::string line;
    while (!is_traced && std::getline(fin, line)) {
        const char kPrefix[] = "TracerPid:\t";
        static constexpr int kPrefixLen = sizeof(kPrefix) - 1;
        if (line.length() > kPrefixLen && line.compare(0, kPrefixLen, kPrefix) == 0) {
            is_traced = line[kPrefixLen] != '0';
        }
    }

    if (is_traced) {
        raise(SIGTRAP);
    }
}
#endif  // platform

#endif  // TINT_ENABLE_BREAK_IN_DEBUGGER

#ifndef TINT_DEBUGGER_BREAK_DEFINED
void tint::debugger::Break() {}
#endif
