// Copyright 2023 The Dawn & Tint Authors
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

#include "dawn/tests/unittests/wire/WireFutureTest.h"

#include "dawn/common/Assert.h"

namespace dawn::wire {

WGPUCallbackMode ToWGPUCallbackMode(CallbackMode callbackMode) {
    switch (callbackMode) {
        case CallbackMode::WaitAny:
            return WGPUCallbackMode_WaitAnyOnly;
        case CallbackMode::ProcessEvents:
            return WGPUCallbackMode_AllowProcessEvents;
        case CallbackMode::Spontaneous:
            return WGPUCallbackMode_AllowSpontaneous;
        default:
            DAWN_UNREACHABLE();
    }
}

std::ostream& operator<<(std::ostream& os, const WireFutureTestParam& param) {
    switch (param.mCallbackMode) {
        case CallbackMode::Async:
            os << "Async";
            break;
        case CallbackMode::WaitAny:
            os << "WaitOnly";
            break;
        case CallbackMode::ProcessEvents:
            os << "ProcessEvents";
            break;
        case CallbackMode::Spontaneous:
            os << "Spontaneous";
            break;
    }
    return os;
}

}  // namespace dawn::wire
