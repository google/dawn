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

// Compatibility shims for linking prebuilt //third_party/llvm-dev
// static archives against targets built using Dawn's hermetic Debian
// Bullseye sysroot.
//
// Why this file is needed:
//   Dawn's GN toolchain compiles and links Linux targets against
//   Chromium's hermetic Debian Bullseye sysroot
//   (//build/linux/debian_bullseye_amd64-sysroot).
//
//   However, the precompiled static archives in
//   `//third_party/llvm-dev/lib/libLLVM*.a` were built on a Linux
//   distribution with newer dependencies (glibc, libstdc++).  As a
//   result, object files inside `libLLVM*.a` reference symbols that
//   do not exist in the Debian Bullseye sysroot.
//
//   This file provides hidden, weak fallback definitions for those 5
//   symbols so that `libvulkan_lvp.so` links cleanly with
//   `-Wl,--no-undefined` against the hermetic sysroot:
//   - `visibility("hidden")` ensures these shims remain strictly
//     internal to `libvulkan_lvp.so` and are never exported into the
//     host process.
//   - `weak` ensures that if Dawn's sysroot or `llvm-dev` package is
//     updated in the future such that a strong definition is provided
//     by the sysroot, the sysroot's definition takes precedence.
//
// TODO(576097100): Once the llvm-dev 3pp is built by Chromium bots
// instead of being a pre-built from upstream this shim should no
// longer be needed

#include <cxxabi.h>
#include <string.h>
#include <exception>

extern "C" {

// Referenced by GCC 11+ <ext/atomicity.h> in prebuilt libLLVM*.a.
// Setting to 0 forces thread-safe atomic refcounting.
__attribute__((visibility("hidden"), weak)) char __libc_single_threaded = 0;

struct mallinfo2_compat {
    size_t arena, ordblks, smblks, hblks, hblkhd, usmblks, fsmblks, uordblks,
        fordblks, keepcost;
};

// Referenced by llvm::sys::Process::GetMallocUsage() in libLLVMSupport.a.
__attribute__((visibility("hidden"), weak)) struct mallinfo2_compat mallinfo2(
    void) {
    struct mallinfo2_compat m;
    memset(&m, 0, sizeof(m));
    return m;
}

}  // extern "C"

namespace std {

// Referenced by GCC 11+ std::vector / std::allocator in libLLVM*.a.
__attribute__((visibility("hidden"), weak, noreturn)) void
__throw_bad_array_new_length() {
    __cxxabiv1::__cxa_throw_bad_array_new_length();
}

namespace __exception_ptr {

// Non-inlined out-of-line refcount helpers referenced by libLLVMRuntimeDyld.a.
__attribute__((visibility("hidden"), weak)) void
exception_ptr::_M_addref() noexcept {}

__attribute__((visibility("hidden"), weak)) void
exception_ptr::_M_release() noexcept {}

}  // namespace __exception_ptr
}  // namespace std
