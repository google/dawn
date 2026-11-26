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

#include "src/dawn/node/standalone/RenderDocCapture.h"

#include <iostream>
#include <string>
#include <utility>

#include "dawn/native/DawnNative.h"
#include "src/dawn/node/binding/GPUDevice.h"

namespace dawn::node::standalone {

Napi::Value RenderDocCapture::StartCapture(const Napi::CallbackInfo& info) {
    auto* capture = static_cast<RenderDocCapture*>(info.Data());
    wgpu::interop::Interface<wgpu::interop::GPUDevice> js_device;
    if (!wgpu::interop::Converter<wgpu::interop::Interface<wgpu::interop::GPUDevice>>::FromJS(
            info.Env(), info[0], js_device)) {
        return Napi::Boolean::New(info.Env(), false);
    }
    const wgpu::Device& device = *js_device.As<wgpu::binding::GPUDevice>();
    if (!dawn::native::StartRenderDocCapture(device.Get())) {
        return Napi::Boolean::New(info.Env(), false);
    }
    capture->capture_device_ = device;
    return Napi::Boolean::New(info.Env(), true);
}

void RenderDocCapture::EndCapture(const Napi::CallbackInfo& info) {
    auto* capture = static_cast<RenderDocCapture*>(info.Data());
    wgpu::Device device = std::exchange(capture->capture_device_, nullptr);
    std::string name = info[0].As<Napi::String>().Utf8Value();
    if (!dawn::native::EndRenderDocCapture(device.Get(), name.c_str())) {
        std::cerr << "[renderdoc] failed to capture " << name << "\n";
    }
}

void RenderDocCapture::Configure(Napi::Env env) {
    Napi::Function start = Napi::Function::New(env, StartCapture, "startCapture", this);
    Napi::Function end = Napi::Function::New(env, EndCapture, "endCapture", this);
    Napi::Value configure_fn = env.RunScript(R"(
        (startCapture, endCapture) => {
            const { globalTestConfig } =
                require('./out-node/common/framework/test_config.js');
            // RenderDoc's Vulkan layer refuses a second simultaneous VkDevice.
            globalTestConfig.oneDevice = true;

            // RenderDoc only captures devices that exist when the capture starts, so each case's
            // capture starts when the case acquires its device.
            let waiting = undefined;
            let capturing = undefined;
            globalTestConfig.testCaseStartCallback = (query) => {
                waiting = query;
            };
            globalTestConfig.deviceAcquiredCallback = (device) => {
                if (waiting !== undefined && startCapture(device)) {
                    capturing = waiting;
                    waiting = undefined;
                }
            };
            globalTestConfig.testCaseEndCallback = () => {
                waiting = undefined;
                if (capturing !== undefined) {
                    endCapture(capturing);
                    capturing = undefined;
                }
            };
        }
    )");
    configure_fn.As<Napi::Function>().Call({start, end});
}

}  // namespace dawn::node::standalone
