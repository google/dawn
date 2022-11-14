// Copyright 2017 The Dawn Authors
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

#include "dawn/common/Assert.h"

#include <cstdlib>

#include "dawn/common/Log.h"
#include "dawn/common/Platform.h"

#if DAWN_COMPILER_IS(CLANG) || DAWN_COMPILER_IS(GCC)
void BreakPoint() {
#if DAWN_PLATFORM_IS(X86)
    __asm__ __volatile__("int $3\n\t");
#elif DAWN_PLATFORM_IS(ARM32)
    __asm__ __volatile__("bkpt 0");
#elif DAWN_PLATFORM_IS(ARM64)
    __asm__ __volatile__("brk 0");
#elif DAWN_PLATFORM_IS(LOONGARCH)
    __asm__ __volatile__("break 0");
#elif DAWN_PLATFORM_IS(RISCV)
    __asm__ __volatile__("ebreak");
#elif DAWN_PLATFORM_IS(MIPS)
    __asm__ __volatile__("break");
#elif DAWN_PLATFORM_IS(S390) || DAWN_PLATFORM_IS_(S390X)
    __asm__ __volatile__(".word 0x0001");
#elif DAWN_PLATFORM_IS(PPC) || DAWN_PLATFORM_IS_(PPC64)
    __asm__ __volatile__("twge 2,2");
#else
#error "Unsupported platform"
#endif
}

#elif DAWN_COMPILER_IS(MSVC)
extern void __cdecl __debugbreak(void);
void BreakPoint() {
    __debugbreak();
}

#else
#error "Unsupported compiler"
#endif

void HandleAssertionFailure(const char* file,
                            const char* function,
                            int line,
                            const char* condition) {
    dawn::ErrorLog() << "Assertion failure at " << file << ":" << line << " (" << function
                     << "): " << condition;
#if defined(DAWN_ABORT_ON_ASSERT)
    abort();
#else
    BreakPoint();
#endif
}
