// Copyright 2017 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_D3D12_D3D12_PLATFORM_H_
#define SRC_DAWN_NATIVE_D3D12_D3D12_PLATFORM_H_

#if defined(INCLUDE_DAWN_NATIVE_D3D12BACKEND_H_)
#error "Include d3d12_platform.h instead of D3D12Backend.h"
#endif

#if defined(__d3d12_h__)
#error "Include d3d12_platform.h before d3d12.h"
#endif

// Dawn's D3D12 implementation must include this header instead of including the public
// D3D12Backend.h directly. This selects the Agility SDK's d3d12.h before any Windows SDK D3D
// header can include its version. D3D12Backend.h then reuses the declarations already selected by
// the d3d12.h include guard. External consumers include D3D12Backend.h directly and use the Windows
// SDK because the public API exposes only stable D3D12 interfaces. Keeping Agility private avoids
// imposing Dawn's SDK checkout, version, include path, and warning suppressions on consumers or
// conflicting with D3D headers they already selected.
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmicrosoft-enum-value"
#pragma clang diagnostic ignored "-Wnested-anon-types"
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#endif
#include "build/native/include/d3d12.h"
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

#include "dawn/native/D3D12Backend.h"
#include "src/dawn/native/d3d/d3d_platform.h"

#endif  // SRC_DAWN_NATIVE_D3D12_D3D12_PLATFORM_H_
